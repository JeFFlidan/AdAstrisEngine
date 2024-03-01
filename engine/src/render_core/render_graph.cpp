#include "render_graph.h"
#include "render_pass.h"
#include "profiler/logger.h"

#include <algorithm>

using namespace ad_astris;
using namespace rcore;
using namespace impl;

RenderGraph::RenderGraph(IRendererResourceManager* rendererResourceManager) : _rendererResourceManager(rendererResourceManager)
{
	
}

void RenderGraph::init(rhi::RHI* engineRHI)
{
	_rhi = engineRHI;
}

void RenderGraph::cleanup()
{
	// TODO
}

IRenderPass* RenderGraph::add_new_pass(const std::string& passName, RenderGraphQueue queue)
{
	auto it = _logicalPassIndexByItsName.find(passName);
	if (it != _logicalPassIndexByItsName.end())
	{
		return _logicalPasses[it->second].get();
	}

	uint32_t passIndex = _logicalPasses.size();
	_logicalPasses.emplace_back(new RenderPass(this, passName, passIndex, queue));
	_logicalPassIndexByItsName[passName] = passIndex;
	return _logicalPasses.back().get();
}

IRenderPass* RenderGraph::get_pass(const std::string& passName)
{
	auto it = _logicalPassIndexByItsName.find(passName);
	if (it == _logicalPassIndexByItsName.end())
		LOG_FATAL("RenderGraph::get_pass(): There is no render pass with name {}", passName)

	return _logicalPasses[it->second].get();
}

void RenderGraph::bake()
{
	clear_collections();
	
	if (_swapChainInputName.empty())
	{
		LOG_ERROR("RenderGraph::bake(): Swap chain doesn't have any input")
		return;
	}
	
	_passDependencies.resize(_logicalPasses.size());

	uint32_t swapChainInputIndex = _logicalResourceIndexByItsName[_swapChainInputName];
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
	
	optimize_pass_order();
	
	build_physical_resources();
	build_rendering_begin_info();
	build_barriers();
}

void RenderGraph::log()
{
	for (auto& passIndex : _sortedPasses)
	{
		RenderPass* pass = _logicalPasses[passIndex].get();
		LOG_INFO("RenderPass name: {}", pass->get_name())
	}

	uint32_t flushingBarrierCount = 0;
	for (auto& pair : _flushingPipelineBarriersByPassIndex)
		flushingBarrierCount += pair.second.size();

	LOG_INFO("RenderGraph::log(): Invalidating barrier count {}", _invalidatingPipelineBarriers.size())
	LOG_INFO("RenderGraph::log(): Flushing barrier count {}", flushingBarrierCount)
}

TextureDesc* RenderGraph::get_texture_desc(const std::string& textureName)
{
	auto it = _logicalResourceIndexByItsName.find(textureName);
	if (it != _logicalResourceIndexByItsName.end())
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
	_logicalResourceIndexByItsName[textureName] = resourceIndex;
	return get_texture_desc_handle(resourceIndex);
}
 
BufferDesc* RenderGraph::get_buffer_desc(const std::string& bufferName)
{
	auto it = _logicalResourceIndexByItsName.find(bufferName);
	if (it != _logicalResourceIndexByItsName.end())
	{
		ResourceDesc* resourceDesc = _logicalResources[it->second].get();
		if (resourceDesc->get_type() != ResourceDesc::Type::BUFFER)
		{
			LOG_ERROR("RenderGraph::get_buffer_desc(): Texture has name {}, not buffer", bufferName)
			return nullptr;
		}
		return get_buffer_desc_handle(resourceDesc);
	}

	uint32_t resourceIndex = _logicalResources.size();
	_logicalResources.emplace_back(new BufferDesc(bufferName, resourceIndex));
	_logicalResourceIndexByItsName[bufferName] = resourceIndex;
	return get_buffer_desc_handle(resourceIndex);
}

void RenderGraph::draw(tasks::TaskGroup* taskGroup)
{
	rhi::CommandBuffer cmd;
	_rhi->begin_command_buffer(&cmd);
	_rhi->add_pipeline_barriers(&cmd, _invalidatingPipelineBarriers);

	for (auto& passIndex : _sortedPasses)
	{
		RenderPass* renderPass = _logicalPasses[passIndex].get();
		if (check_if_graphics(renderPass))
			_rhi->begin_rendering(&cmd, &_renderingBeginInfoByPassIndex[passIndex]);
		
		renderPass->get_executor()->execute(&cmd);

		if (check_if_graphics(renderPass))
			_rhi->end_rendering(&cmd);
		
		auto it = _flushingPipelineBarriersByPassIndex.find(passIndex);
		if (it != _flushingPipelineBarriersByPassIndex.end())
			_rhi->add_pipeline_barriers(&cmd, it->second);
	}

	rhi::ClearValues clearValues;
	clearValues.color = { 0.0f, 0.0f, 0.0f, 1.0f };
	_rhi->begin_rendering(&cmd, nullptr, &clearValues);
	_swapChainExecutor->execute(&cmd);
	_rhi->end_rendering(&cmd, nullptr);
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

void RenderGraph::build_rendering_begin_info()
{
	for (auto& passIndex : _sortedPasses)
	{
		RenderPass* renderPass = _logicalPasses[passIndex].get();
		if (check_if_compute(renderPass))
			continue;

		rhi::RenderingBeginInfo& beginInfo = _renderingBeginInfoByPassIndex[passIndex];
		uint32_t viewCount = renderPass->get_view_count();
		if (viewCount)
		{
			beginInfo.multiviewInfo.isEnabled = true;
			beginInfo.multiviewInfo.viewCount = viewCount;
		}
		TextureDesc* depthTextureDesc = renderPass->get_depth_stencil_output();
		if (depthTextureDesc)
		{
			rhi::RenderTarget& renderTarget = beginInfo.renderTargets.emplace_back();
			renderTarget.target = get_physical_texture_view(depthTextureDesc->get_name());
			renderTarget.clearValue.depthStencil.depth = 1.0f;
			renderTarget.loadOp = rhi::LoadOp::CLEAR;
			renderTarget.storeOp = rhi::StoreOp::STORE;
		}

		for (auto& colorTextureDesc : renderPass->get_color_outputs())
		{
			rhi::RenderTarget& renderTarget = beginInfo.renderTargets.emplace_back();
			renderTarget.target = get_physical_texture_view(colorTextureDesc->get_name());
			renderTarget.clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderTarget.loadOp = rhi::LoadOp::CLEAR;
			renderTarget.storeOp = rhi::StoreOp::STORE;
		}
	}
}

void RenderGraph::build_barriers()
{
	std::unordered_map<uint32_t, uint32_t> incompleteBarrierByResIndex;
	std::unordered_set<uint32_t> resourcesWithFlushingBarrier;
	std::unordered_set<uint32_t> resourcesWithInvalidatingBarrier;
	
	for (auto& passIndex : _sortedPasses)
	{
		RenderPass* logicalPass = _logicalPasses[passIndex].get();
		uint32_t logicalPassIndex = logicalPass->get_logical_pass_index();

		{
			TextureDesc* depthTextureDesc = logicalPass->get_depth_stencil_input();
			if (depthTextureDesc)
			{
				auto it = resourcesWithFlushingBarrier.find(depthTextureDesc->get_logical_index());
				if (it == resourcesWithFlushingBarrier.end())
				{
					LOG_FATAL("RenderGraph::bake(): Pass {} can't have texture {} as depth input because this texture hasn't been written", logicalPass->get_name(), depthTextureDesc->get_name())
				}
			}
		}

		{
			TextureDesc* depthTextureDesc = logicalPass->get_depth_stencil_output();
			if (depthTextureDesc)
			{
				rhi::PipelineBarrier& invalidatingBarrier = _invalidatingPipelineBarriers.emplace_back();
				rhi::PipelineBarrier& flushingBarrier = _flushingPipelineBarriersByPassIndex[passIndex].emplace_back();
				invalidatingBarrier = rhi::PipelineBarrier::set_texture_barrier(
					get_physical_texture(depthTextureDesc->get_name()),
					rhi::ResourceLayout::UNDEFINED,
					rhi::ResourceLayout::DEPTH_STENCIL);
				flushingBarrier = rhi::PipelineBarrier::set_texture_barrier(
					get_physical_texture(depthTextureDesc->get_name()),
					rhi::ResourceLayout::DEPTH_STENCIL,
					rhi::ResourceLayout::SHADER_READ);
				resourcesWithFlushingBarrier.insert(depthTextureDesc->get_logical_index());
				resourcesWithInvalidatingBarrier.insert(depthTextureDesc->get_logical_index());
			}
		}

		for (auto& colorInput : logicalPass->get_color_inputs())
		{
			auto it = resourcesWithFlushingBarrier.find(colorInput->get_logical_index());
			if (it == resourcesWithFlushingBarrier.end())
			{
				LOG_FATAL("RenderGraph::bake(): Pass {} can't have texture {} as color input because this texture hasn't been written", logicalPass->get_name(), colorInput->get_name())
			}
		}
		
		for (auto& colorOutput : logicalPass->get_color_outputs())
		{
			bool isBlitSrc = false;
			for (auto& blitTexture : logicalPass->get_blit_textures())
			{
				TextureDesc* blitSrcTexture = blitTexture.first;
				if (colorOutput->get_logical_index() == blitSrcTexture->get_logical_index())
				{
					isBlitSrc = true;
					break;
				}
			}

			rhi::PipelineBarrier& invalidatingBarrier = _invalidatingPipelineBarriers.emplace_back();
			invalidatingBarrier = rhi::PipelineBarrier::set_texture_barrier(
				get_physical_texture(colorOutput->get_name()),
				rhi::ResourceLayout::UNDEFINED,
				rhi::ResourceLayout::COLOR_ATTACHMENT);
			resourcesWithInvalidatingBarrier.insert(colorOutput->get_logical_index());
			
			if (isBlitSrc)
				continue;
		
			rhi::PipelineBarrier& flushingBarrier = _flushingPipelineBarriersByPassIndex[passIndex].emplace_back();
			flushingBarrier = rhi::PipelineBarrier::set_texture_barrier(
				get_physical_texture(colorOutput->get_name()),
				rhi::ResourceLayout::COLOR_ATTACHMENT,
				rhi::ResourceLayout::SHADER_READ);
			resourcesWithFlushingBarrier.insert(colorOutput->get_logical_index());
		}

		// for (auto& textureDesc : logicalPass->get_custom_textures())
		// {
		// 	// TODO 
		// }

		for (auto& storageTextureDesc : logicalPass->get_storage_texture_inputs())
		{
			auto it = resourcesWithInvalidatingBarrier.find(storageTextureDesc->get_logical_index());
			if (it == resourcesWithInvalidatingBarrier.end())
			{
				LOG_FATAL("RenderGraph::bake(): Pass {} can't have texture {} as storage input because this texture hasn't been written", logicalPass->get_name(), storageTextureDesc->get_name())
			}
		}

		for (auto& storageTextureDesc : logicalPass->get_storage_texture_outputs())
		{
			auto it = resourcesWithInvalidatingBarrier.find(storageTextureDesc->get_logical_index());
			if (it != resourcesWithInvalidatingBarrier.end())
				continue;

			rhi::PipelineBarrier& invalidatingBarrier = _invalidatingPipelineBarriers[passIndex];
			invalidatingBarrier = rhi::PipelineBarrier::set_texture_barrier(
				_rendererResourceManager->get_texture_view(storageTextureDesc->get_name())->texture,
				rhi::ResourceLayout::UNDEFINED,
				rhi::ResourceLayout::GENERAL);
			resourcesWithInvalidatingBarrier.insert(storageTextureDesc->get_logical_index());
		}
		
		for (auto& blitTextures : logicalPass->get_blit_textures())
		{
			TextureDesc* srcTextureDesc = blitTextures.first;
			TextureDesc* dstTextureDesc = blitTextures.second;
		
			rhi::PipelineBarrier& srcBeforeBarrier = _beforeBlitPipelineBarriersByPassIndex[passIndex].emplace_back();
			rhi::PipelineBarrier& dstBeforeBarrier = _beforeBlitPipelineBarriersByPassIndex[passIndex].emplace_back();
			rhi::PipelineBarrier& srcAfterBarrier = _afterBlitPipelineBarriersByPassIndex[passIndex].emplace_back();
			rhi::PipelineBarrier& dstAfterBarrier = _afterBlitPipelineBarriersByPassIndex[passIndex].emplace_back();
		
			srcBeforeBarrier = rhi::PipelineBarrier::set_texture_barrier(
				get_physical_texture(srcTextureDesc->get_name()),
				rhi::ResourceLayout::COLOR_ATTACHMENT,
				rhi::ResourceLayout::TRANSFER_SRC); // TODO

			dstBeforeBarrier = rhi::PipelineBarrier::set_texture_barrier(
				get_physical_texture(dstTextureDesc->get_name()),
				rhi::ResourceLayout::UNDEFINED,
				rhi::ResourceLayout::TRANSFER_DST);

			srcAfterBarrier = rhi::PipelineBarrier::set_texture_barrier(
				get_physical_texture(srcTextureDesc->get_name()),
				rhi::ResourceLayout::TRANSFER_SRC,
				rhi::ResourceLayout::SHADER_READ);

			dstAfterBarrier = rhi::PipelineBarrier::set_texture_barrier(
				get_physical_texture(dstTextureDesc->get_name()),
				rhi::ResourceLayout::TRANSFER_DST,
				rhi::ResourceLayout::SHADER_READ);

			resourcesWithFlushingBarrier.insert(srcTextureDesc->get_logical_index());
			resourcesWithFlushingBarrier.insert(dstTextureDesc->get_logical_index());
		}

		// for (auto& storageBufferDesc : logicalPass->get_storage_buffer_inputs())
		// {
		// 	// TODO Think how to sync storage buffers
		// }
		//
		for (auto& storageBufferDesc : logicalPass->get_storage_buffer_outputs())
		{
			rhi::Buffer* buffer = get_physical_buffer(storageBufferDesc->get_name());
			if (has_flag(buffer->bufferInfo.bufferUsage, rhi::ResourceUsage::INDIRECT_BUFFER))
			{
				rhi::PipelineBarrier& flushingBarrier = _flushingPipelineBarriersByPassIndex[passIndex].emplace_back();
				flushingBarrier = rhi::PipelineBarrier::set_buffer_barrier(
					buffer,
					rhi::ResourceLayout::SHADER_WRITE,
					rhi::ResourceLayout::INDIRECT_COMMAND_BUFFER);
			}
		}
		//
		// for (auto& transferBufferDesc : logicalPass->get_transfer_outputs())
		// {
		// 	// TODO Think how to sync transfer buffers
		// }

		// for (auto& indirectBufferDesc : logicalPass->get_indirect_buffer_inputs())
		// {
		// 	// TODO probably have to remove this loop
		// }
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
				break;
			case ResourceDesc::Type::TEXTURE:
				create_physical_texture(reinterpret_cast<TextureDesc*>(resource));
				break;
		}
	}
}

void RenderGraph::create_physical_texture(TextureDesc* logicalTexture)
{
	if (logicalTexture->has_texture_info())
	{
		rhi::TextureInfo& textureInfo = logicalTexture->get_texture_info();
		_rendererResourceManager->allocate_texture(logicalTexture->get_name(), textureInfo);
		_rendererResourceManager->allocate_texture_view(logicalTexture->get_name(), logicalTexture->get_name());
	}
}

void RenderGraph::create_physical_buffer(BufferDesc* logicalBuffer)
{
	if (logicalBuffer->has_buffer_info())
	{
		rhi::BufferInfo& bufferInfo = logicalBuffer->get_buffer_info();
		_rendererResourceManager->allocate_buffer(logicalBuffer->get_name(), bufferInfo);
	}
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

rhi::TextureView* RenderGraph::get_physical_texture_view(const std::string& name)
{
	return _rendererResourceManager->get_texture_view(name);
}

rhi::Texture* RenderGraph::get_physical_texture(const std::string& name)
{
	return _rendererResourceManager->get_texture_view(name)->texture;
}

rhi::Buffer* RenderGraph::get_physical_buffer(const std::string& name)
{
	return _rendererResourceManager->get_buffer(name);
}

void RenderGraph::clear_collections()
{
	_sortedPasses.clear();
	_passDependencies.clear();
	_invalidatingPipelineBarriers.clear();
	_flushingPipelineBarriersByPassIndex.clear();
	_beforeBlitPipelineBarriersByPassIndex.clear();
	_afterBlitPipelineBarriersByPassIndex.clear();
}
