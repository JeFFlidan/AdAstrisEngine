#include "render_pass.h"

using namespace ad_astris;
using namespace rcore;
using namespace impl;

RenderPass::RenderPass(IRenderGraph* renderGraph, const std::string& name, uint32_t index, RenderGraphQueue queue)
	: _renderGraph(renderGraph), _name(name), _logicalIndex(index), _queue(queue)
{
	
}

RenderPass::~RenderPass()
{
	
}

TextureDesc* RenderPass::set_depth_stencil_input(const std::string& inputName)
{
	TextureDesc* desc = _renderGraph->get_texture_desc(inputName);
	desc->add_read_in_pass(_logicalIndex);
	_depthStencilInput = desc;
	return desc;
}

TextureDesc* RenderPass::set_depth_stencil_output(const std::string& outputName, rhi::TextureInfo* textureInfo)
{
	TextureDesc* desc = _renderGraph->get_texture_desc(outputName);
	desc->set_texture_info(textureInfo);
	desc->add_queue(_queue);
	desc->add_texture_usage(rhi::ResourceUsage::DEPTH_STENCIL_ATTACHMENT);
	desc->add_written_in_pass(_logicalIndex);
	_depthStencilOutput = desc;
	return desc;
}

TextureDesc* RenderPass::add_color_input(const std::string& inputName)
{
	TextureDesc* desc = _renderGraph->get_texture_desc(inputName);
	desc->add_read_in_pass(_logicalIndex);
	desc->add_queue(_queue);
	desc->add_texture_usage(rhi::ResourceUsage::SAMPLED_TEXTURE);
	_colorInputs.push_back(desc);
	return desc;
}

TextureDesc* RenderPass::add_color_output(const std::string& outputName, rhi::TextureInfo* textureInfo)
{
	TextureDesc* desc = _renderGraph->get_texture_desc(outputName);
	desc->set_texture_info(textureInfo);
	desc->add_queue(_queue);
	desc->add_written_in_pass(_logicalIndex);
	desc->add_texture_usage(rhi::ResourceUsage::COLOR_ATTACHMENT);
	_colorOutputs.push_back(desc);
	return desc;
}

TextureDesc* RenderPass::add_history_input(const std::string& inputName)
{
	TextureDesc* desc = _renderGraph->get_texture_desc(inputName);
	desc->add_read_in_pass(_logicalIndex);
	desc->add_texture_usage(rhi::ResourceUsage::SAMPLED_TEXTURE);
	desc->add_queue(_queue);
	_historyInputs.push_back(desc);
	return desc;
}

TextureDesc* RenderPass::add_texture_input(
	const std::string& inputName,
	rhi::TextureInfo* textureInfo,
	rhi::ShaderType shaderStages)
{
	TextureDesc* desc = _renderGraph->get_texture_desc(inputName);
	
	define_shader_stages(desc, shaderStages);

	desc->add_read_in_pass(_logicalIndex);
	desc->add_queue(_queue);
	desc->add_texture_usage(rhi::ResourceUsage::SAMPLED_TEXTURE);
	desc->set_texture_info(textureInfo);
	_customTextures.push_back(desc);
	
	return desc;
}

TextureDesc* RenderPass::add_storage_texture_input(const std::string& inputName)
{
	TextureDesc* desc = _renderGraph->get_texture_desc(inputName);
	desc->add_queue(_queue);
	desc->add_read_in_pass(_logicalIndex);
	desc->add_texture_usage(rhi::ResourceUsage::STORAGE_TEXTURE);
	_storageTextureInputs.push_back(desc);
	return desc;
}

TextureDesc* RenderPass::add_storage_texture_output(const std::string& outputName, rhi::TextureInfo* textureInfo)
{
	TextureDesc* desc = _renderGraph->get_texture_desc(outputName);
	desc->add_queue(_queue);
	desc->add_written_in_pass(_logicalIndex);
	desc->set_texture_info(textureInfo);
	desc->add_texture_usage(rhi::ResourceUsage::STORAGE_TEXTURE);
	_storageTextureOutputs.push_back(desc);
	return desc;
}

std::pair<TextureDesc*, TextureDesc*>& RenderPass::add_texture_blit_input_and_output(
	const std::string& inputName,
	const std::string& outputName,
	rhi::TextureInfo* outputTextureInfo)
{
	TextureDesc* inputDesc = _renderGraph->get_texture_desc(inputName);
	inputDesc->add_texture_usage(rhi::ResourceUsage::TRANSFER_SRC);
	inputDesc->add_read_in_pass(_logicalIndex);
	inputDesc->add_queue(_queue);

	TextureDesc* outputDesc = _renderGraph->get_texture_desc(outputName);
	outputDesc->set_texture_info(outputTextureInfo);
	outputDesc->add_queue(_queue);
	outputDesc->add_written_in_pass(_logicalIndex);
	outputDesc->add_texture_usage(rhi::ResourceUsage::TRANSFER_DST);

	_blitTextureInputsAndOutputs.push_back({ inputDesc, outputDesc });
	return _blitTextureInputsAndOutputs.back();
}

BufferDesc* RenderPass::add_uniform_buffer_input(
	const std::string& inputName,
	rhi::BufferInfo* bufferInfo,
	rhi::ShaderType shaderStages)
{
	BufferDesc* desc = _renderGraph->get_buffer_desc(inputName);
	
	desc->set_buffer_info(bufferInfo);
	desc->add_queue(_queue);
	desc->add_read_in_pass(_logicalIndex);
	desc->add_buffer_usage(rhi::ResourceUsage::UNIFORM_BUFFER);
	define_shader_stages(desc, shaderStages);
	
	_uniformBufferInputs.push_back(desc);
	return desc;
}

BufferDesc* RenderPass::add_storage_buffer_read_only_input(const std::string& inputName, rhi::ShaderType shaderStages)
{
	BufferDesc* desc = _renderGraph->get_buffer_desc(inputName);
	
	desc->add_queue(_queue);
	desc->add_read_in_pass(_logicalIndex);
	desc->add_buffer_usage(rhi::ResourceUsage::STORAGE_BUFFER);
	define_shader_stages(desc, shaderStages);
	
	_storageBufferInputs.push_back(desc);
	return desc;
}

BufferDesc* RenderPass::add_storage_buffer_read_write_output(const std::string& outputName, rhi::BufferInfo* bufferInfo)
{
	BufferDesc* desc = _renderGraph->get_buffer_desc(outputName);
	desc->set_buffer_info(bufferInfo);
	desc->add_queue(_queue);
	desc->add_written_in_pass(_logicalIndex);
	desc->add_buffer_usage(rhi::ResourceUsage::STORAGE_BUFFER);
	_storageBufferOutputs.push_back(desc);
	return desc;
}

BufferDesc* RenderPass::add_transfer_output(const std::string& outputName, rhi::BufferInfo* bufferInfo)
{
	BufferDesc* desc = _renderGraph->get_buffer_desc(outputName);
	desc->set_buffer_info(bufferInfo);
	desc->add_queue(_queue);
	desc->add_written_in_pass(_logicalIndex);
	desc->add_buffer_usage(rhi::ResourceUsage::TRANSFER_DST);
	_transferOutputs.push_back(desc);
	return desc;
}

BufferDesc* RenderPass::add_vertex_buffer_input(const std::string& inputName)
{
	BufferDesc* desc = _renderGraph->get_buffer_desc(inputName);
	desc->add_queue(_queue);
	desc->add_read_in_pass(_logicalIndex);
	desc->add_buffer_usage(rhi::ResourceUsage::VERTEX_BUFFER);
	_vertexBufferInputs.push_back(desc);
	return desc;
}

BufferDesc* RenderPass::add_index_buffer_input(const std::string& inputName)
{
	BufferDesc* desc = _renderGraph->get_buffer_desc(inputName);
	desc->add_queue(_queue);
	desc->add_read_in_pass(_logicalIndex);
	desc->add_buffer_usage(rhi::ResourceUsage::INDEX_BUFFER);
	_indexBufferInputs.push_back(desc);
	return desc;
}

BufferDesc* RenderPass::add_indirect_buffer_input(const std::string& inputName)
{
	BufferDesc* desc = _renderGraph->get_buffer_desc(inputName);
	desc->add_queue(_queue);
	desc->add_read_in_pass(_logicalIndex);
	desc->add_buffer_usage(rhi::ResourceUsage::INDIRECT_BUFFER);
	_indirectBufferInputs.push_back(desc);
	return desc;
}

void RenderPass::define_shader_stages(ResourceDesc* desc, rhi::ShaderType shaderStages)
{
	if (shaderStages != rhi::ShaderType::UNDEFINED)
	{
		desc->add_shader_stage(shaderStages);
	}
	else
	{
		if (_queue == RenderGraphQueue::GRAPHICS || _queue == RenderGraphQueue::ASYNC_GRAPHICS)
			desc->add_shader_stage(rhi::ShaderType::FRAGMENT);
		else if (_queue == RenderGraphQueue::COMPUTE || _queue == RenderGraphQueue::ASYNC_COMPUTE)
			desc->add_shader_stage(rhi::ShaderType::COMPUTE);
	}
}
