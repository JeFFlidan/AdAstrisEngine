#include "render_graph.h"
#include "render_pass.h"
#include "profiler/logger.h"

using namespace ad_astris;
using namespace rcore;
using namespace impl;

RenderGraph::RenderGraph(rhi::IEngineRHI* engineRHI) : _engineRHI(engineRHI)
{

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

void RenderGraph::log()
{
	RenderPass* pass = _logicalPasses.back().get();
	LOG_INFO("Color outputs size: {}", pass->get_color_outputs().size())
	LOG_INFO("Custom textures size: {}", pass->get_custom_textures().size())
	LOG_INFO("Storage texture inputs: {}", pass->get_storage_texture_inputs().size())
	LOG_INFO("Resources count: {}", _logicalResources.size())
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

	return _physicalTextureViews[physicalIndex];
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

	return _physicalBuffers[physicalIndex];
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
