#include "render_graph.h"
#include "render_pass.h"
#include "profiler/logger.h"

#include <algorithm>

using namespace ad_astris;
using namespace rcore;
using namespace impl;

RenderGraph::RenderGraph(rhi::IEngineRHI* engineRHI) : _engineRHI(engineRHI)
{

}

void RenderGraph::cleanup()
{
	// TODO
}

IRenderPass* RenderGraph::add_new_pass(const std::string& passName, RenderGraphQueue queue)
{
	auto it = _logicalPassNameToIndex.find(passName);
	if (it != _logicalPassNameToIndex.end())
	{
		return _logicalPasses[it->second].get();
	}

	uint32_t passIndex = _logicalPasses.size();
	_logicalPasses.emplace_back(new RenderPass(this, passName, passIndex, queue));
	return _logicalPasses.back().get();
}

IRenderPass* RenderGraph::get_pass(const std::string& passName)
{
	auto it = _logicalPassNameToIndex.find(passName);
	if (it == _logicalPassNameToIndex.end())
		LOG_FATAL("RenderGraph::get_pass(): There is no render pass with name {}", passName)

	return _logicalPasses[it->second].get();
}

void RenderGraph::bake()
{
	_sortedPasses.clear();
	_passBarriers.clear();
	
	if (_swapChainInputName.empty())
	{
		LOG_ERROR("RenderGraph::bake(): Swap chain doesn't have any input")
		return;
	}
	
	_passDependencies.resize(_logicalPasses.size());

	uint32_t swapChainInputIndex = _logicalResourceNameToIndex[_swapChainInputName];
	ResourceDesc* swapChainInputRes = _logicalResources[swapChainInputIndex].get();

	for (auto& passIndex : swapChainInputRes->get_written_in_passes())
	{
		_sortedPasses.push_back(passIndex);
	}
	
	auto tempSortedPasses = _sortedPasses;
	
	for (auto& passIndex : tempSortedPasses)
	{
		RenderPass* pass = _logicalPasses[passIndex].get();
		solve_graph(pass, 0);
	}

	std::reverse(_sortedPasses.begin(), _sortedPasses.end());
	filter_pass_order();

	_passBarriers.resize(_sortedPasses.size());

	LOG_INFO("Before optimizing pass order")
	optimize_pass_order();

	LOG_INFO("Before building physical resources")
	build_physical_resources();
	LOG_INFO("Before building physical passes")
	build_physical_passes();
	LOG_INFO("Before building barriers")
	build_barriers();
	LOG_INFO("After building barriers")
	_resourceNameToLastLayout.clear();
}

void RenderGraph::log()
{
	// RenderPass* pass = _logicalPasses.back().get();
	// LOG_INFO("Color outputs size: {}", pass->get_color_outputs().size())
	// LOG_INFO("Custom textures size: {}", pass->get_custom_textures().size())
	// LOG_INFO("Storage texture inputs: {}", pass->get_storage_texture_inputs().size())
	// LOG_INFO("Resources count: {}", _logicalResources.size())
	
	for (auto& passIndex : _sortedPasses)
	{
		RenderPass* pass = _logicalPasses[passIndex].get();
		LOG_INFO("RenderPass name: {}", pass->get_name())
	}
	
	LOG_INFO("RenderGraph::log(): Physical passes amount: {}", _physicalPasses.size())
	LOG_INFO("RenderGraph::log(): Barriers amount: {}", _passBarriers.size())
}

TextureDesc* RenderGraph::get_texture_desc(const std::string& textureName)
{
	auto it = _logicalResourceNameToIndex.find(textureName);
	if (it != _logicalResourceNameToIndex.end())
	{
		ResourceDesc* resourceDesc = _logicalResources[it->second].get();
		if (resourceDesc->get_type() != ResourceDesc::Type::TEXTURE)
		{
			LOG_ERROR("RenderGraph::get_texture_desc(): Buffer has name {}, not texture", textureName)
			return nullptr;
		}
		return get_texture_desc_handle(resourceDesc);
	}

	uint32_t resourceIndex = _logicalResources.size();
	_logicalResources.emplace_back(new TextureDesc(textureName, resourceIndex));
	_logicalResourceNameToIndex[textureName] = resourceIndex;
	return get_texture_desc_handle(resourceIndex);
}
 
BufferDesc* RenderGraph::get_buffer_desc(const std::string& bufferName)
{
	auto it = _logicalResourceNameToIndex.find(bufferName);
	if (it != _logicalResourceNameToIndex.end())
	{
		ResourceDesc* resourceDesc = _logicalResources[it->second].get();
		if (resourceDesc->get_type() != ResourceDesc::Type::TEXTURE)
		{
			LOG_ERROR("RenderGraph::get_buffer_desc(): Texture has name {}, not buffer", bufferName)
			return nullptr;
		}
		return get_buffer_desc_handle(resourceDesc);
	}

	uint32_t resourceIndex = _logicalResources.size();
	_logicalResources.emplace_back(new BufferDesc(bufferName, resourceIndex));
	_logicalResourceNameToIndex[bufferName] = resourceIndex;
	return get_buffer_desc_handle(resourceIndex);
}
		
rhi::TextureView* RenderGraph::get_physical_texture(TextureDesc* textureDesc)
{
	return get_physical_texture(textureDesc->get_physical_index());
}

rhi::TextureView* RenderGraph::get_physical_texture(const std::string& textureName)
{
	auto it = _logicalResourceNameToIndex.find(textureName);
	if (it == _logicalResourceNameToIndex.end())
	{
		LOG_ERROR("RenderGraph::get_physical_texture(): Can't get physical texture because there is no logical texture with name {}", textureName)
		return nullptr;
	}
	if (_logicalResources[it->second]->get_type() != ResourceDesc::Type::TEXTURE)
	{
		LOG_ERROR("RenderGraph::get_physical_texture(): Buffer has name {}, not texture", textureName)
		return nullptr;
	}

	return get_physical_texture(get_texture_desc_handle(it->second)->get_physical_index());
}

rhi::TextureView* RenderGraph::get_physical_texture(uint32_t physicalIndex)
{
	if (physicalIndex > _physicalTextureViews.size())
	{
		LOG_ERROR("RenderGraph::get_physical_texture(): Physical index can't be greater than physical texture views amount ({} > {})", physicalIndex, _physicalTextureViews.size())
		return nullptr;
	}
	if (physicalIndex == ResourceDesc::Unused)
	{
		LOG_ERROR("RenderGraph::get_physical_texture(): Can't get physical index because logical texture doesn't have an associated physical texture view")
		return nullptr;
	}

	return &_physicalTextureViews[physicalIndex];
}

rhi::Buffer* RenderGraph::get_physical_buffer(BufferDesc* bufferDesc)
{
	return get_physical_buffer(bufferDesc->get_physical_index());
}

rhi::Buffer* RenderGraph::get_physical_buffer(const std::string& bufferName)
{
	auto it = _logicalResourceNameToIndex.find(bufferName);
	if (it == _logicalResourceNameToIndex.end())
	{
		LOG_ERROR("RenderGraph::get_physical_buffer(): You can't get physical texture because there is no logical buffer with name {}", bufferName)
		return nullptr;
	}
	if (_logicalResources[it->second]->get_type() != ResourceDesc::Type::BUFFER)
	{
		LOG_ERROR("RenderGraph::get_physical_texture(): Texture has name {}, not buffer", bufferName)
		return nullptr;
	}

	return get_physical_buffer(get_buffer_desc_handle(it->second)->get_physical_index());
}

rhi::Buffer* RenderGraph::get_physical_buffer(uint32_t physicalIndex)
{
	if (physicalIndex > _physicalTextureViews.size())
	{
		LOG_ERROR("RenderGraph::get_physical_buffer(): Physical index can't be greater than physical buffers amount ({} > {})", physicalIndex, _physicalTextureViews.size())
		return nullptr;
	}
	if (physicalIndex == ResourceDesc::Unused)
	{
		LOG_ERROR("RenderGraph::get_physical_buffer(): Can't get physical index because logical buffer doesn't have an associated physical buffer")
		return nullptr;
	}

	return &_physicalBuffers[physicalIndex];
}

void RenderGraph::solve_graph(RenderPass* passHandle, uint32_t passesInStackCount)
{
	for (auto& colorInput : passHandle->get_color_inputs())
	{
		parse_passes_recursively(passHandle, colorInput->get_written_in_passes(), passesInStackCount, true, false);
	}
	
	for (auto& texture : passHandle->get_custom_textures())
	{
		parse_passes_recursively(passHandle, texture->get_written_in_passes(), passesInStackCount, false, false);
	}

	for (auto& storageTexture : passHandle->get_storage_texture_inputs())
	{
		parse_passes_recursively(passHandle, storageTexture->get_written_in_passes(), passesInStackCount, true, false);
	}

	for (auto& blitTextures : passHandle->get_blit_textures())
	{
		TextureDesc* inputBlit = blitTextures.first;
		parse_passes_recursively(passHandle, inputBlit->get_written_in_passes(), passesInStackCount, true, false);
	}

	for (auto& storageBuffer : passHandle->get_storage_buffer_inputs())
	{
		parse_passes_recursively(passHandle, storageBuffer->get_written_in_passes(), passesInStackCount, true, false);
	}

	for (auto& indirectBuffer : passHandle->get_index_buffer_inputs())
	{
		parse_passes_recursively(passHandle, indirectBuffer->get_written_in_passes(), passesInStackCount, true, false);
	}
}

void RenderGraph::parse_passes_recursively(
	RenderPass* currentPass,
	std::unordered_set<uint32_t>& writtenPasses,
	uint32_t passesInStackCount,
	bool checkIfEmpty,
	bool skipIfSelfDependency)
{
	if (checkIfEmpty && writtenPasses.empty())
	{
		LOG_ERROR("RenderGraph::bake(): Resource isn't written anywhere")
		return;
	}
	if (passesInStackCount > _logicalPasses.size())
	{
		LOG_ERROR("RenderGraph::bake(): There is a cycle in the graph")
		return;
	}

	for (auto& passIndex : writtenPasses)
	{
		if (passIndex != currentPass->get_logical_pass_index())
			_passDependencies[currentPass->get_logical_pass_index()].insert(passIndex);
	}

	++passesInStackCount;

	for (auto& passIndex : writtenPasses)
	{
		RenderPass* tempPass = _logicalPasses[passIndex].get();
		if (skipIfSelfDependency && tempPass->get_logical_pass_index() == currentPass->get_logical_pass_index())
		{
			continue;
		}
		else if (tempPass->get_logical_pass_index() == currentPass->get_logical_pass_index())
		{
			LOG_ERROR("RenderGraph::bake(): Pass {} depends on itself", currentPass->get_name())
			return;
		}

		_sortedPasses.push_back(passIndex);
		solve_graph(tempPass, passesInStackCount);
	}
}

void RenderGraph::filter_pass_order()
{
	std::unordered_set<uint32_t> indicesCache;
	std::vector<uint32_t> filteredOrder;
	
	for (int i = 0; i != _sortedPasses.size(); ++i)
	{
		uint32_t passIndex = _sortedPasses[i];
		
		if (indicesCache.find(passIndex) == indicesCache.end())
		{
			filteredOrder.push_back(passIndex);
		}
		indicesCache.insert(passIndex);
	}

	_sortedPasses.resize(filteredOrder.size());
	memcpy(_sortedPasses.data(), filteredOrder.data(), filteredOrder.size() * sizeof(uint32_t));
}

void RenderGraph::optimize_pass_order()
{
	
}

void RenderGraph::build_barriers()
{
	for (auto& passIndex : _sortedPasses)
	{
		RenderPass* logicalPass = _logicalPasses[passIndex].get();

		{
			TextureDesc* depthTextureDesc = logicalPass->get_depth_stencil_input();

			if (depthTextureDesc)
			{
				auto it = _resourceNameToLastLayout.find(depthTextureDesc->get_name());

				if (it == _resourceNameToLastLayout.end())
				{
					LOG_FATAL("RenderGraph::bake(): Depth stencil input {} hasn't been written", depthTextureDesc->get_name())
				}
				else
				{
					rhi::ResourceLayout lastLayout = it->second;
					if (lastLayout != rhi::ResourceLayout::SHADER_READ)
					{
						setup_texture_barrier(logicalPass, depthTextureDesc, lastLayout, rhi::ResourceLayout::SHADER_READ);
					}
				}
			}
		}

		{
			TextureDesc* depthTextureDesc = logicalPass->get_depth_stencil_output();
			if (depthTextureDesc)
				_resourceNameToLastLayout[depthTextureDesc->get_name()] = rhi::ResourceLayout::DEPTH_STENCIL;
		}

		for (auto& colorInput : logicalPass->get_color_inputs())
		{
			auto it = _resourceNameToLastLayout.find(colorInput->get_name());

			if (it == _resourceNameToLastLayout.end())
			{
				LOG_FATAL("RenderGraph::bake(): Color input {} was not written", colorInput->get_name())
			}

			rhi::ResourceLayout lastLayout = it->second;

			if (lastLayout != rhi::ResourceLayout::SHADER_READ)
			{
				setup_texture_barrier(logicalPass, colorInput, lastLayout, rhi::ResourceLayout::SHADER_READ);
			}
		}

		for (auto& colorOutput : logicalPass->get_color_outputs())
		{
			_resourceNameToLastLayout[colorOutput->get_name()] = rhi::ResourceLayout::COLOR_ATTACHMENT;
		}

		for (auto& textureDesc : logicalPass->get_custom_textures())
		{
			auto it = _resourceNameToLastLayout.find(textureDesc->get_name());

			rhi::ResourceLayout lastLayout = it == _resourceNameToLastLayout.end() ? rhi::ResourceLayout::UNDEFINED : it->second;
			
			if (lastLayout != rhi::ResourceLayout::SHADER_READ)
			{
				setup_texture_barrier(logicalPass, textureDesc, lastLayout, rhi::ResourceLayout::SHADER_READ);
			}
		}

		for (auto& storageTextureDesc : logicalPass->get_storage_texture_inputs())
		{
			auto it = _resourceNameToLastLayout.find(storageTextureDesc->get_name());

			if (it == _resourceNameToLastLayout.end())
			{
				LOG_FATAL("RenderGraph::bake(): Pass {} can't have storage texture {} as input because this texture hasn't been written before", logicalPass->get_name(), storageTextureDesc->get_name())
			}

			rhi::ResourceLayout lastLayout = it->second;
			if (lastLayout != rhi::ResourceLayout::GENERAL)
			{
				setup_texture_barrier(logicalPass, storageTextureDesc, lastLayout, rhi::ResourceLayout::GENERAL);
			}
		}

		for (auto& storageTextureDesc : logicalPass->get_storage_texture_outputs())
		{
			auto it = _resourceNameToLastLayout.find(storageTextureDesc->get_name());
			auto endIt = _resourceNameToLastLayout.end();

			rhi::ResourceLayout lastLayout = it == endIt ? rhi::ResourceLayout::UNDEFINED : it->second;

			if (lastLayout != rhi::ResourceLayout::GENERAL)
			{
				LOG_INFO("RenderGraph::bake(): Output storage texture {} doesn't have GENERAL layout", storageTextureDesc->get_name())

				setup_texture_barrier(logicalPass, storageTextureDesc, lastLayout, rhi::ResourceLayout::GENERAL);
			}
		}

		for (auto& blitTextures : logicalPass->get_blit_textures())
		{
			TextureDesc* srcTextureDesc = blitTextures.first;
			TextureDesc* dstTextureDesc = blitTextures.second;

			auto srcIt = _resourceNameToLastLayout.find(srcTextureDesc->get_name());
			auto dstIt = _resourceNameToLastLayout.find(dstTextureDesc->get_name());
			auto endIt = _resourceNameToLastLayout.end();
			
			rhi::ResourceLayout srcTextureLastLayout = srcIt == endIt ? rhi::ResourceLayout::UNDEFINED : srcIt->second;
			rhi::ResourceLayout dstTextureLastLayout = dstIt == endIt ? rhi::ResourceLayout::UNDEFINED : dstIt->second;

			if (srcTextureLastLayout != rhi::ResourceLayout::TRANSFER_SRC)
			{
				LOG_INFO("RenderGraph::bake(): Blit src not transfer src")

				setup_texture_barrier(logicalPass, srcTextureDesc, srcTextureLastLayout, rhi::ResourceLayout::TRANSFER_SRC);
			}

			if (dstTextureLastLayout != rhi::ResourceLayout::TRANSFER_DST)
			{
				LOG_INFO("RenderGraph::bake(): Blit dst not transfer dst")

				setup_texture_barrier(logicalPass, dstTextureDesc, dstTextureLastLayout, rhi::ResourceLayout::TRANSFER_DST);
			}
		}

		for (auto& storageBufferDesc : logicalPass->get_storage_buffer_inputs())
		{
			auto it = _resourceNameToLastLayout.find(storageBufferDesc->get_name());

			if (it == _resourceNameToLastLayout.end())
			{
				LOG_FATAL("RenderGraph::bake(): Input storage buffer {} has not been written", storageBufferDesc->get_name())
			}

			rhi::ResourceLayout lastLayout = it->second;
			if (!has_flag(lastLayout, rhi::ResourceLayout::SHADER_READ))
			{
				setup_buffer_barrier(logicalPass, storageBufferDesc, lastLayout, rhi::ResourceLayout::SHADER_READ);
			}
		}

		for (auto& storageBufferDesc : logicalPass->get_storage_buffer_outputs())
		{
			auto it = _resourceNameToLastLayout.find(storageBufferDesc->get_name());
			auto endIt = _resourceNameToLastLayout.end();

			rhi::ResourceLayout readFlag = rhi::ResourceLayout::SHADER_READ;
			rhi::ResourceLayout writeFlag = rhi::ResourceLayout::SHADER_WRITE;
			rhi::ResourceLayout readWriteFlags = readFlag | writeFlag;
			rhi::ResourceLayout lastLayout = it == endIt ? readWriteFlags : it->second;

			if (!has_flag(lastLayout, readFlag) || !has_flag(lastLayout, writeFlag))
			{
				setup_buffer_barrier(logicalPass, storageBufferDesc, lastLayout, readWriteFlags);
			}
		}

		for (auto& transferBufferDesc : logicalPass->get_transfer_outputs())
		{
			auto it = _resourceNameToLastLayout.find(transferBufferDesc->get_name());
			auto endIt = _resourceNameToLastLayout.end();

			if (it == endIt)
			{
				_resourceNameToLastLayout[transferBufferDesc->get_name()] = rhi::ResourceLayout::TRANSFER_DST;
			}
			else if (!has_flag(it->second, rhi::ResourceLayout::TRANSFER_DST))
			{
				rhi::ResourceLayout lastLayout = it->second;
				setup_buffer_barrier(logicalPass, transferBufferDesc, lastLayout, rhi::ResourceLayout::TRANSFER_DST);
			}
		}

		for (auto& indirectBufferDesc : logicalPass->get_indirect_buffer_inputs())
		{
			auto it = _resourceNameToLastLayout.find(indirectBufferDesc->get_name());
			auto endIt = _resourceNameToLastLayout.end();

			if (it == endIt)
			{
				LOG_FATAL("RenderGraph::bake(): Indirect buffer {} has not been written", indirectBufferDesc->get_name())
			}

			rhi::ResourceLayout lastLayout = it->second;

			if (!has_flag(lastLayout, rhi::ResourceLayout::INDIRECT_COMMAND_BUFFER))
			{
				setup_buffer_barrier(logicalPass, indirectBufferDesc, lastLayout, rhi::ResourceLayout::INDIRECT_COMMAND_BUFFER);
			}
		}
	}
}

void RenderGraph::build_physical_resources()
{
	for (auto& resourceUniquePtr : _logicalResources)
	{
		ResourceDesc* resource = resourceUniquePtr.get();

		switch (resource->get_type())
		{
			case ResourceDesc::Type::BUFFER:
				create_physical_buffer(reinterpret_cast<BufferDesc*>(resource));
			case ResourceDesc::Type::TEXTURE:
				create_physical_texture(reinterpret_cast<TextureDesc*>(resource));
		}
	}
}

void RenderGraph::build_physical_passes()
{
	for (auto& logicalPassIndex : _sortedPasses)
	{
		RenderPass* logicalPass = _logicalPasses[logicalPassIndex].get();

		if (logicalPass->get_physical_pass_index() != ResourceDesc::Unused)
			continue;

		rhi::RenderPassInfo physPassInfo;
		setup_physical_pass_queue(physPassInfo, logicalPass);

		if (logicalPass->is_multiview_enabled())
		{
			rhi::MultiviewInfo multiviewInfo;
			multiviewInfo.isEnabled = true;
			if (logicalPass->get_depth_stencil_output())
			{
				multiviewInfo.viewCount = logicalPass->get_depth_stencil_output()->get_texture_info().layersCount;
			}
			else
			{
				multiviewInfo.viewCount = logicalPass->get_color_outputs()[0]->get_texture_info().layersCount;
			}
			
			physPassInfo.multiviewInfo = multiviewInfo;
		}

		{
			TextureDesc* depthStencilOutput = logicalPass->get_depth_stencil_output();
			if (depthStencilOutput)
			{
				rhi::TextureView& textureView = _physicalTextureViews[depthStencilOutput->get_physical_index()];
		
				rhi::RenderTarget target;
				setup_depth_stencil_render_target(target, textureView);

				physPassInfo.renderTargets.push_back(target);
			}
		}
		
		for (auto& colorOutput : logicalPass->get_color_outputs())
		{
			rhi::TextureView& textureView = _physicalTextureViews[colorOutput->get_physical_index()];
		
			rhi::RenderTarget target;
			setup_color_render_target(target, textureView);

			physPassInfo.renderTargets.push_back(target);
		}

		rhi::RenderPass physicalPass;
		_engineRHI->create_render_pass(&physicalPass, &physPassInfo);
		uint32_t physicalPassIndex = _physicalPasses.size();
		_physicalPasses.push_back(physicalPass);
		logicalPass->set_physical_pass_index(physicalPassIndex);
	}
}

void RenderGraph::create_physical_texture(TextureDesc* logicalTexture)
{
	rhi::TextureInfo& textureInfo = logicalTexture->get_texture_info();
	_physicalTextures.push_back(rhi::Texture());
	rhi::Texture* texture = &_physicalTextures.back();
	_engineRHI->create_texture(texture, &textureInfo);
	
	rhi::TextureViewInfo viewInfo;
	viewInfo.baseLayer = 0;
	viewInfo.baseMipLevel = 0;
	rhi::TextureView textureView;
	_engineRHI->create_texture_view(&textureView, &viewInfo, texture);

	uint32_t viewIndex = _physicalTextureViews.size();
	_physicalTextureViews.push_back(textureView);

	logicalTexture->set_physical_index(viewIndex);
}

void RenderGraph::create_physical_buffer(BufferDesc* logicalBuffer)
{
	uint32_t physicalIndex = _physicalBuffers.size();
	logicalBuffer->set_physical_index(physicalIndex);
	
	_physicalBuffers.push_back(rhi::Buffer());
	rhi::Buffer* buffer = &_physicalBuffers.back();

	rhi::BufferInfo& bufferInfo = logicalBuffer->get_buffer_info();
	_engineRHI->create_buffer(buffer, &bufferInfo);
}

void RenderGraph::setup_physical_pass_queue(rhi::RenderPassInfo& physPassInfo, RenderPass* logicalPass)
{
	RenderGraphQueue queue = logicalPass->get_queue();
	if (has_flag(queue, RenderGraphQueue::GRAPHICS) || has_flag(queue, RenderGraphQueue::ASYNC_GRAPHICS))
	{
		physPassInfo.pipelineType = rhi::PipelineType::GRAPHICS;
	}
	else if (has_flag(queue, RenderGraphQueue::COMPUTE) || has_flag(queue, RenderGraphQueue::ASYNC_COMPUTE))
	{
		physPassInfo.pipelineType = rhi::PipelineType::COMPUTE;
	}
}

void RenderGraph::setup_depth_stencil_render_target(rhi::RenderTarget& renderTarget, rhi::TextureView& textureView)
{
	renderTarget.target = &textureView;
	renderTarget.type = rhi::RenderTargetType::DEPTH;
	renderTarget.loadOp = rhi::LoadOp::CLEAR;
	renderTarget.storeOp = rhi::StoreOp::STORE;
	renderTarget.initialLayout = rhi::ResourceLayout::UNDEFINED;
	renderTarget.renderPassLayout = rhi::ResourceLayout::DEPTH_STENCIL;
	renderTarget.finalLayout = rhi::ResourceLayout::DEPTH_STENCIL;
}

void RenderGraph::setup_color_render_target(rhi::RenderTarget& renderTarget, rhi::TextureView& textureView)
{
	renderTarget.target = &textureView;
	renderTarget.type = rhi::RenderTargetType::COLOR;
	renderTarget.loadOp = rhi::LoadOp::CLEAR;
	renderTarget.storeOp = rhi::StoreOp::STORE;
	renderTarget.initialLayout = rhi::ResourceLayout::UNDEFINED;
	renderTarget.renderPassLayout = rhi::ResourceLayout::COLOR_ATTACHMENT;
	renderTarget.finalLayout = rhi::ResourceLayout::COLOR_ATTACHMENT;
}

void RenderGraph::setup_texture_barrier(RenderPass* passHandle, TextureDesc* textureDesc, rhi::ResourceLayout srcLayout, rhi::ResourceLayout dstLayout)
{
	rhi::Texture* textureHandle = get_physical_texture(textureDesc)->texture;
	
	rhi::PipelineBarrier barrier = rhi::PipelineBarrier::set_texture_barrier(textureHandle, srcLayout, dstLayout);
	_passBarriers[passHandle->get_logical_pass_index()].push_back(barrier);

	_resourceNameToLastLayout[textureDesc->get_name()] = dstLayout;
}

void RenderGraph::setup_buffer_barrier(RenderPass* passHandle, BufferDesc* bufferDesc, rhi::ResourceLayout srcLayout, rhi::ResourceLayout dstLayout)
{
	rhi::Buffer* bufferHandle = get_physical_buffer(bufferDesc);

	rhi::PipelineBarrier barrier = rhi::PipelineBarrier::set_buffer_barrier(bufferHandle, srcLayout, dstLayout);
	_passBarriers[passHandle->get_logical_pass_index()].push_back(barrier);

	_resourceNameToLastLayout[passHandle->get_name()] = dstLayout;
}

bool RenderGraph::check_if_compute(RenderPass* passHandle)
{
	RenderGraphQueue queue = passHandle->get_queue();
	return has_flag(queue, RenderGraphQueue::COMPUTE) || has_flag(queue, RenderGraphQueue::ASYNC_COMPUTE);
}

bool RenderGraph::check_if_graphics(RenderPass* passHandle)
{
	RenderGraphQueue queue = passHandle->get_queue();
	return has_flag(queue, RenderGraphQueue::GRAPHICS) || has_flag(queue, RenderGraphQueue::ASYNC_GRAPHICS);
}


TextureDesc* RenderGraph::get_texture_desc_handle(uint32_t index)
{
	return reinterpret_cast<TextureDesc*>(_logicalResources[index].get());
}

TextureDesc* RenderGraph::get_texture_desc_handle(ResourceDesc* resourceDesc)
{
	return reinterpret_cast<TextureDesc*>(resourceDesc);
}

BufferDesc* RenderGraph::get_buffer_desc_handle(uint32_t index)
{
	return reinterpret_cast<BufferDesc*>(_logicalResources[index].get());
}

BufferDesc* RenderGraph::get_buffer_desc_handle(ResourceDesc* resourceDesc)
{
	return reinterpret_cast<BufferDesc*>(resourceDesc);
}
