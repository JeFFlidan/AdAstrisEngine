#include <iostream>

#include <cmath>
#include "engine_actors.h"
#include "fmt/core.h"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "logger.h"
#include "material_system.h"
#include "vk_descriptors.h"
#include "vk_engine.h"
#include "vk_initializers.h"
#include "vk_scene.h"
#include "vk_types.h"
#include <memory>
#include <stdint.h>
#include <vulkan/vulkan_core.h>
#include <future>
#include <algorithm>
#include <set>

using GeometryInfo = TransparencyFirstPassData::GeometryInfo;
using Node = TransparencyFirstPassData::Node;

inline uint32_t get_group_count(uint32_t threadCount, uint32_t localSize)
{
	return (threadCount + localSize - 1) / localSize;
}

inline glm::vec4 normalize_plane(glm::vec4 p)
{
	return p / glm::length(glm::vec3(p));
}

inline glm::mat4 perspectiveProjection(float fovY, float aspectWbyH, float zNear)
{
	float f = 1.0f / std::tan(fovY / 2.0f);
	return glm::mat4(
		f / aspectWbyH, 0.0f, 0.0f, 0.0f,
		0.0f, f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, zNear, 0.0f);
}

void VulkanEngine::fill_renderable_objects()
{
	_renderScene.register_object_batch(_meshObjects.data(), _meshObjects.size());
	_renderScene.build_batches();
	_renderScene.merge_meshes(this);
}

void VulkanEngine::draw_deferred_pass(VkCommandBuffer cmd)
{
	auto& meshPass = _renderScene._deferredPass;

	CullParams forwardPassCullParams;
	forwardPassCullParams.frustumCull = true;
	forwardPassCullParams.occlusionCull = true;
	forwardPassCullParams.drawDist = 9999999;
	forwardPassCullParams.aabb = false;
	forwardPassCullParams.viewMatrix = camera.get_view_matrix();
	forwardPassCullParams.projMatrix = camera.get_projection_matrix(_windowExtent.width, _windowExtent.height, true);
	
	culling(meshPass, cmd, forwardPassCullParams);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		0, 0, nullptr, _afterCullingBufferBarriers.size(), _afterCullingBufferBarriers.data(), 0, nullptr);

	_afterCullingBufferBarriers.clear();

	VkDescriptorImageInfo samplerInfo{};
	samplerInfo.sampler = _textureSampler;

	VkDescriptorBufferInfo objectDataInfo = _renderScene._objectDataBuffer.get_info(true);
	VkDescriptorBufferInfo instanceInfo = meshPass.compactedInstanceBuffer.get_info(true);
	VkDescriptorBufferInfo cameraInfo = get_current_frame()._cameraBuffer.get_info();
	VkDescriptorBufferInfo settingsInfo = get_current_frame()._settingsBuffer.get_info();
	VkDescriptorBufferInfo taaInfo = _temporalFilter.jitteringBuffer.get_info();

	VkDescriptorSet globalDescriptorSet;
	VkDescriptorSet texturesDescriptorSet1;
	VkDescriptorSet texturesDescriptorSet2;
	VkDescriptorSet texturesDescriptorSet3;
	
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &objectDataInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT))
		.bind_buffer(1, &instanceInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(2, &cameraInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(3, &settingsInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(4, &taaInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_image(5, &samplerInfo, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(globalDescriptorSet);

	size_t descriptorsAmount = _renderScene._baseColorInfos.size() + _renderScene._normalInfos.size() + _renderScene._armInfos.size();
	
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._baseColorInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._baseColorInfos.size())
		.build_non_uniform(texturesDescriptorSet1, _renderScene._baseColorInfos.size());
		
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._normalInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._normalInfos.size())
		.build_non_uniform(texturesDescriptorSet2, _renderScene._normalInfos.size());

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._armInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._armInfos.size())
		.build_non_uniform(texturesDescriptorSet3, _renderScene._armInfos.size());

	std::vector<VkDescriptorSet> sets = {
		globalDescriptorSet,
		texturesDescriptorSet1,
		texturesDescriptorSet2,
		texturesDescriptorSet3
	};

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_renderScene._globalVertexBuffer._buffer, &offset);
	vkCmdBindIndexBuffer(cmd, _renderScene._globalIndexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);
	
	std::array<VkClearValue, 5> clearValues;
	clearValues[0].color = { { 0.15f, 0.15f, 0.15f, 1.0f } };
	clearValues[1].color = { { 0.15f, 0.15f, 0.15f, 1.0f } };
	clearValues[2].color = { { 0.15f, 0.15f, 0.15f, 1.0f } };
	clearValues[3].color = { { 0.15f, 0.15f, 0.15f, 1.0f } };
	clearValues[4].depthStencil.depth = 1.0f;

	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_GBuffer.renderPass, _windowExtent, _GBuffer.framebuffer);
	rpInfo.clearValueCount = 5;
	
	rpInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(get_current_frame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.width = static_cast<float>(_windowExtent.width);
	viewport.height = static_cast<float>(_windowExtent.height);
	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = _windowExtent;
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	vkutil::ShaderPass* prevMaterial = nullptr;
	
	for (int i = 0; i != meshPass.multibatches.size(); ++i)
	{
		auto& multibatch = meshPass.multibatches[i];
		auto& batch = meshPass.batches[multibatch.first];

		if (batch.material.shaderPass->relatedShaderPasses.empty())
		{
			LOG_FATAL("There is no GBuffer shader pass");
		}
		auto shaderPass = batch.material.shaderPass->relatedShaderPasses[0];

		if (shaderPass != prevMaterial)
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
			for (int i = 0; i != sets.size(); ++i)
			{
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->layout, i, 1, &sets[i], 0, nullptr);
			}
			prevMaterial = shaderPass;
		}

		VkDeviceSize offset = multibatch.first * sizeof(GPUIndirectObject);
		uint32_t stride = sizeof(GPUIndirectObject);

		vkCmdDrawIndexedIndirect(cmd, meshPass.drawIndirectBuffer._buffer, offset, multibatch.count, stride);
	}

	vkCmdEndRenderPass(get_current_frame()._mainCommandBuffer);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	VkMemoryBarrier memoryBarrier{};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1,&memoryBarrier, 0, nullptr, 0, nullptr);

	VkImageMemoryBarrier depthImgBarrier{};
	depthImgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	depthImgBarrier.image = _GBuffer.depth.imageData.image;
	depthImgBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthImgBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	depthImgBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	depthImgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	depthImgBarrier.subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 };

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &depthImgBarrier
	);
	
	VkDescriptorImageInfo attachSamplerInfo{};
	attachSamplerInfo.sampler = _linearSampler;
	VkDescriptorImageInfo shadowSamplerInfo{};
	shadowSamplerInfo.sampler = _shadowMapSampler;

	VkDescriptorImageInfo gAlbedoInfo{};
	gAlbedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	gAlbedoInfo.imageView = _GBuffer.albedo.imageView;
	VkDescriptorImageInfo gNormalInfo{};
	gNormalInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	gNormalInfo.imageView = _GBuffer.normal.imageView;
	VkDescriptorImageInfo gSurfaceInfo{};
	gSurfaceInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	gSurfaceInfo.imageView = _GBuffer.surface.imageView;
	VkDescriptorImageInfo gDepthInfo{};
	gDepthInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	gDepthInfo.imageView = _GBuffer.depth.imageView;
	VkDescriptorBufferInfo sceneInfo = get_current_frame()._sceneDataBuffer.get_info();
	VkDescriptorBufferInfo dirLightsInfo = _renderScene._dirLightsBuffer.get_info(true);
	VkDescriptorBufferInfo pointLightsInfo = _renderScene._pointLightsBuffer.get_info(true);
	VkDescriptorBufferInfo spotLigthsInfo = _renderScene._spotLightsBuffer.get_info(true);

	VkDescriptorSet globalDescriptorSet2;
	VkDescriptorSet testSet;
	VkDescriptorSet dirShadowMapsDescriptorSet;
	VkDescriptorSet pointShadowMapsDescriptorSet;
	VkDescriptorSet spotShadowMapsDescriptorSet;
	
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &sceneInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(1, &cameraInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(2, &dirLightsInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(3, &pointLightsInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(4, &spotLigthsInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(5, &gAlbedoInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(6, &gNormalInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(7, &gSurfaceInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(8, &gDepthInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(9, &attachSamplerInfo, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(10, &shadowSamplerInfo, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(globalDescriptorSet);

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._dirShadowMapsInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._dirShadowMaps.size())
		.build_non_uniform(dirShadowMapsDescriptorSet, _renderScene._dirShadowMapsInfos.size());

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._pointShadowMapsInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._pointShadowMapsInfos.size())
		.build_non_uniform(pointShadowMapsDescriptorSet, _renderScene._pointShadowMapsInfos.size());

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._spotShadowMapsInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._spotShadowMapsInfos.size())
		.build_non_uniform(spotShadowMapsDescriptorSet, _renderScene._spotShadowMapsInfos.size());

	std::vector<VkDescriptorSet> descriptorSets = {
		globalDescriptorSet,
		//testSet
		dirShadowMapsDescriptorSet,
		pointShadowMapsDescriptorSet,
		spotShadowMapsDescriptorSet
	};

	rpInfo = vkinit::renderpass_begin_info(_deferredRenderPass, _windowExtent, _deferredFramebuffer);
	VkClearValue clearValue;
	clearValue.color = { { 0.15f, 0.15f, 0.15f, 1.0f } };
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	auto material = _materialSystem.get_material("wall");

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	if (material != nullptr)
	{
		auto shaderPass = material->original->passShaders[vkutil::MeshpassType::Deferred];
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
		for (int i = 0; i != descriptorSets.size(); ++i)
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->layout, i, 1, &descriptorSets[i], 0, nullptr);
	}
	else
	{
		LOG_FATAL("Invalid material for deferred lighting");
	}

	offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_outputQuad._vertexBuffer._buffer, &offset);
	vkCmdDraw(cmd, _outputQuad._vertices.size(), 1, 0, 0);

	vkCmdEndRenderPass(cmd);
}

void VulkanEngine::draw_forward_pass(VkCommandBuffer cmd, uint32_t swapchainImageIndex)
{
	auto& meshPass = _renderScene._forwardPass;

	CullParams forwardPassCullParams;
	forwardPassCullParams.frustumCull = true;
	forwardPassCullParams.occlusionCull = true;
	forwardPassCullParams.drawDist = 9999999;
	forwardPassCullParams.aabb = false;
	forwardPassCullParams.viewMatrix = camera.get_view_matrix();
	forwardPassCullParams.projMatrix = camera.get_projection_matrix(_windowExtent.width, _windowExtent.height, true);
	
	culling(meshPass, cmd, forwardPassCullParams);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		0, 0, nullptr, _afterCullingBufferBarriers.size(), _afterCullingBufferBarriers.data(), 0, nullptr);

	_afterCullingBufferBarriers.clear();

	VkDescriptorImageInfo samplerInfo{};
	samplerInfo.sampler = _textureSampler;

	VkDescriptorImageInfo sampler2Info{};
	sampler2Info.sampler = _shadowMapSampler;

	VkDescriptorBufferInfo objectDataInfo = _renderScene._objectDataBuffer.get_info(true);
	VkDescriptorBufferInfo instanceInfo = meshPass.compactedInstanceBuffer.get_info(true);
	VkDescriptorBufferInfo cameraInfo = get_current_frame()._cameraBuffer.get_info();
	VkDescriptorBufferInfo sceneDataInfo = get_current_frame()._sceneDataBuffer.get_info();
	VkDescriptorBufferInfo dirLightsInfo = _renderScene._dirLightsBuffer.get_info(true);
	VkDescriptorBufferInfo pointLightsInfo = _renderScene._pointLightsBuffer.get_info(true);
	VkDescriptorBufferInfo spotLightsInfo = _renderScene._spotLightsBuffer.get_info(true);

	VkDescriptorSet globalDescriptorSet;
	VkDescriptorSet objectDescriptorSet;
	VkDescriptorSet texturesDescriptorSet1;
	VkDescriptorSet texturesDescriptorSet2;
	VkDescriptorSet texturesDescriptorSet3;
	VkDescriptorSet dirShadowMapsDescriptorSet;
	VkDescriptorSet pointShadowMapsDescriptorSet;
	VkDescriptorSet spotShadowMapsDescriptorSet;

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &dirLightsInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(1, &pointLightsInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(2, &spotLightsInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(3, &cameraInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(4, &sceneDataInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(5, &samplerInfo, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(6, &sampler2Info, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(globalDescriptorSet);

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &objectDataInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, (VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT))
		.bind_buffer(1, &instanceInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.build(objectDescriptorSet);

	size_t descriptorsAmount = _renderScene._baseColorInfos.size() + _renderScene._normalInfos.size() + _renderScene._armInfos.size();
	
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._baseColorInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._baseColorInfos.size())
		.build_non_uniform(texturesDescriptorSet1, _renderScene._baseColorInfos.size());
		
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._normalInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._normalInfos.size())
		.build_non_uniform(texturesDescriptorSet2, _renderScene._normalInfos.size());

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._armInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._armInfos.size())
		.build_non_uniform(texturesDescriptorSet3, _renderScene._armInfos.size());

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._dirShadowMapsInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._dirShadowMaps.size())
		.build_non_uniform(dirShadowMapsDescriptorSet, _renderScene._dirShadowMapsInfos.size());

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._pointShadowMapsInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._pointShadowMapsInfos.size())
		.build_non_uniform(pointShadowMapsDescriptorSet, _renderScene._pointShadowMapsInfos.size());

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, _renderScene._spotShadowMapsInfos.data(), VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT, 100, _renderScene._spotShadowMapsInfos.size())
		.build_non_uniform(spotShadowMapsDescriptorSet, _renderScene._spotShadowMapsInfos.size());

	std::vector<VkDescriptorSet> sets = {
		globalDescriptorSet,
		objectDescriptorSet,
		texturesDescriptorSet1,
		texturesDescriptorSet2,
		texturesDescriptorSet3,
		dirShadowMapsDescriptorSet,
		pointShadowMapsDescriptorSet,
		spotShadowMapsDescriptorSet};

	vkutil::ShaderPass* prevMaterial = nullptr;
	
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_renderScene._globalVertexBuffer._buffer, &offset);
	vkCmdBindIndexBuffer(cmd, _renderScene._globalIndexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);
	
	VkClearValue clearValue;
	clearValue.color = { { 0.15f, 0.15f, 0.15f, 1.0f } };

	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;
	
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_mainOpaqueRenderPass, _windowExtent, _mainOpaqueFramebuffers[swapchainImageIndex]);
	rpInfo.clearValueCount = 2;
	
	VkClearValue clearValues[] = { clearValue, depthClear };
	rpInfo.pClearValues = clearValues;

	vkCmdBeginRenderPass(get_current_frame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.width = static_cast<float>(_windowExtent.width);
	viewport.height = static_cast<float>(_windowExtent.height);
	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = _windowExtent;
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);
	
	for (int i = 0; i != meshPass.multibatches.size(); ++i)
	{
		auto& multibatch = meshPass.multibatches[i];
		auto& batch = meshPass.batches[multibatch.first];

		auto shaderPass = batch.material.shaderPass;

		if (shaderPass != prevMaterial)
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
			for (int i = 0; i != sets.size(); ++i)
			{
				vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->layout, i, 1, &sets[i], 0, nullptr);
			}
			prevMaterial = shaderPass;
		}

		VkDeviceSize offset = multibatch.first * sizeof(GPUIndirectObject);
		uint32_t stride = sizeof(GPUIndirectObject);

		auto& meshPass = _renderScene._forwardPass;

		vkCmdDrawIndexedIndirect(cmd, meshPass.drawIndirectBuffer._buffer, offset, multibatch.count, stride);
	}

	vkCmdEndRenderPass(get_current_frame()._mainCommandBuffer);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
}

void VulkanEngine::draw_tranparency_pass(VkCommandBuffer cmd)
{
	vkCmdFillBuffer(cmd, _transparencyData.geometryInfo._buffer, 0, sizeof(uint32_t), 0);

	VkImageSubresourceRange subresRange{};
	subresRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresRange.levelCount = 1;
	subresRange.layerCount = 1;

	VkClearColorValue headImageClearColor;
	headImageClearColor.uint32[0] = 0xffffffff;
	vkCmdClearColorImage(cmd, _transparencyData.headIndex.imageData.image, VK_IMAGE_LAYOUT_GENERAL, &headImageClearColor, 1, &subresRange);

	VkMemoryBarrier memBarrier{};
	memBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	memBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &memBarrier, 0, nullptr, 0, nullptr);

	auto& meshPass = _renderScene._transparentForwardPass;

	CullParams transpPassCullParams;
	transpPassCullParams.frustumCull = true;
	transpPassCullParams.occlusionCull = true;
	transpPassCullParams.drawDist = 9999999;
	transpPassCullParams.aabb = false;
	transpPassCullParams.viewMatrix = camera.get_view_matrix();
	transpPassCullParams.projMatrix = camera.get_projection_matrix(_windowExtent.width, _windowExtent.height, true);

	culling(meshPass, cmd, transpPassCullParams);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
		0, 0, nullptr, _afterCullingBufferBarriers.size(), _afterCullingBufferBarriers.data(), 0, nullptr);

	_afterCullingBufferBarriers.clear();

	VkDescriptorBufferInfo cameraInfo = get_current_frame()._cameraBuffer.get_info();
	VkDescriptorBufferInfo objectDataInfo = _renderScene._objectDataBuffer.get_info(true);
	VkDescriptorBufferInfo instanceInfo = meshPass.compactedInstanceBuffer.get_info(true);
	VkDescriptorBufferInfo nodesInfo = _transparencyData.nodes.get_info(true);
	VkDescriptorBufferInfo geometryDataInfo = _transparencyData.geometryInfo.get_info(true);

	VkDescriptorImageInfo headIndexImgInfo;
	headIndexImgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	headIndexImgInfo.imageView = _transparencyData.headIndex.imageView;
	headIndexImgInfo.sampler = nullptr;

	VkDescriptorImageInfo mainOpaqueColorImgInfo;
	mainOpaqueColorImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	mainOpaqueColorImgInfo.imageView = _deferredColorAttach.imageView;
	mainOpaqueColorImgInfo.sampler = _linearSampler;

	VkDescriptorSet oitGeomDescriptorSet;

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &cameraInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(1, &objectDataInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(2, &instanceInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(3, &nodesInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(4, &geometryDataInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(5, &headIndexImgInfo, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(oitGeomDescriptorSet);

	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_transparencyData.renderPass, _windowExtent, _transparencyData.framebuffer);
	rpInfo.clearValueCount = 0;
	rpInfo.pClearValues = nullptr;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.width = static_cast<float>(_windowExtent.width);
	viewport.height = static_cast<float>(_windowExtent.height);
	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = _windowExtent;
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	vkutil::ShaderPass* shaderPass = _transparencyData.geometryPass;
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->layout, 0, 1, &oitGeomDescriptorSet, 0, nullptr);

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_renderScene._globalVertexBuffer._buffer, &offset);
	vkCmdBindIndexBuffer(cmd, _renderScene._globalIndexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);

	std::vector<glm::vec4> colors = {
		glm::vec4(0.1f, 0.9f, 0.1f, 0.4f),
		glm::vec4(0.9f, 0.1f, 0.1f, 0.55f)
	};

	for (int i = 0; i != meshPass.multibatches.size(); ++i)
	{
		auto& multibatch = meshPass.multibatches[i];

		VkDeviceSize offset = multibatch.first * sizeof(GPUIndirectObject);
		uint32_t stride = sizeof(GPUIndirectObject);

		glm::vec4 color = colors[i];
		vkCmdPushConstants(cmd, shaderPass->layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(glm::vec4), &color);

		vkCmdDrawIndexedIndirect(cmd, meshPass.drawIndirectBuffer._buffer, offset, multibatch.count, stride);
	}

	vkCmdEndRenderPass(cmd);
	
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);

	VkMemoryBarrier memoryBarrier{};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1,&memoryBarrier, 0, nullptr, 0, nullptr);

	VkDescriptorBufferInfo settingsInfo = get_current_frame()._settingsBuffer.get_info();
	VkDescriptorBufferInfo taaInfo = _temporalFilter.jitteringBuffer.get_info();
	
	VkDescriptorSet globalSet;

	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &objectDataInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(1, &instanceInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(2, &nodesInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_buffer(3, &cameraInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(4, &settingsInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(5, &taaInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_image(6, &headIndexImgInfo, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(7, &mainOpaqueColorImgInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(globalSet);

	std::array<VkClearValue, 3> clearValues;
	clearValues[0].color = { { 0.15f, 0.15f, 0.15f, 1.0f } };
	clearValues[1].color = { { 0.15f, 0.15f, 0.15f, 1.0f } };
	clearValues[2].depthStencil.depth = 1.0f;
	
	rpInfo = vkinit::renderpass_begin_info(_transparencyRenderPass, _windowExtent, _transparencyFramebuffer);
	rpInfo.clearValueCount = 3;
	rpInfo.pClearValues = clearValues.data();
	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkutil::ShaderPass* prevMaterial = nullptr;

	vkCmdBindVertexBuffers(cmd, 0, 1, &_renderScene._globalVertexBuffer._buffer, &offset);
	vkCmdBindIndexBuffer(cmd, _renderScene._globalIndexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);	

	for (int i = 0; i != meshPass.multibatches.size(); ++i)
	{
		auto& multibatch = meshPass.multibatches[i];
		auto& batch = meshPass.batches[multibatch.first];

		auto shaderPass = batch.material.shaderPass;

		if (shaderPass != prevMaterial)
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->layout, 0, 1, &globalSet, 0, nullptr);
			prevMaterial = shaderPass;
		}

		VkDeviceSize offset = multibatch.first * sizeof(GPUIndirectObject);
		uint32_t stride = sizeof(GPUIndirectObject);

		vkCmdDrawIndexedIndirect(cmd, meshPass.drawIndirectBuffer._buffer, offset, multibatch.count, stride);
	}

	vkCmdEndRenderPass(cmd);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
}

void VulkanEngine::draw_compositing_pass(VkCommandBuffer cmd)
{
	std::array<VkClearValue, 2> clearValues;
	clearValues[0].color = { { 0.15f, 0.15f, 0.15f, 1.0f } };
	clearValues[1].color = { { 0.15f, 0.15f, 0.15f, 1.0f } };

	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_composite.renderPass, _windowExtent, _composite.framebuffer);
	rpInfo.clearValueCount = 2;
	rpInfo.pClearValues = clearValues.data();
	
	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	auto material = _materialSystem.get_material("Composite");
	
	VkDescriptorImageInfo deferredImgInfo;
	deferredImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	deferredImgInfo.imageView = _deferredColorAttach.imageView;

	VkDescriptorImageInfo GBufferVelocityInfo;
	GBufferVelocityInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	GBufferVelocityInfo.imageView = _GBuffer.velocity.imageView;

	VkDescriptorImageInfo GBufferDepthImgInfo;
	GBufferDepthImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	GBufferDepthImgInfo.imageView = _GBuffer.depth.imageView;

	VkDescriptorImageInfo transpColorImgInfo;
	transpColorImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	transpColorImgInfo.imageView = _transparencyColorAttach.imageView;

	VkDescriptorImageInfo transpVelocityInfo;
	transpVelocityInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	transpVelocityInfo.imageView = _transparencyVelocityAttach.imageView;

	VkDescriptorImageInfo transpDepthImgInfo;
	transpDepthImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	transpDepthImgInfo.imageView = _transparencyDepthAttach.imageView;

	VkDescriptorImageInfo nearestSampInfo;
	nearestSampInfo.sampler = _nearestSampler;

	VkDescriptorSet globalSet;
	
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, &deferredImgInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(1, &transpColorImgInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(2, &GBufferVelocityInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(3, &transpVelocityInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(4, &GBufferDepthImgInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(5, &transpDepthImgInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(6, &nearestSampInfo, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(globalSet);
		
	if (material != nullptr)
	{
		auto meshPass = material->original->passShaders[vkutil::MeshpassType::Forward];
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPass->pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPass->layout, 0, 1, &globalSet, 0, nullptr);
	}
	else
	{
		LOG_FATAL("Invalid material for output quad, draw_output_quad func")
	}

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_outputQuad._vertexBuffer._buffer, &offset);
	vkCmdDraw(cmd, _outputQuad._vertices.size(), 1, 0, 0);

	vkCmdEndRenderPass(cmd);
}

void VulkanEngine::draw_taa_pass(VkCommandBuffer cmd)
{
	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr, 0, nullptr);
	
	VkClearValue clearValue;
	clearValue.color = { { 0.15f, 0.15f, 0.15f, 1.0f } };

	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_temporalFilter.taaRenderPass, _windowExtent, _temporalFilter.taaFramebuffer);
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	auto material = _materialSystem.get_material("TAA");

	VkDescriptorImageInfo currentColorInfo;
	currentColorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	currentColorInfo.imageView = _composite.colorAttach.imageView;
	VkDescriptorImageInfo oldTaaInfo;
	oldTaaInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	oldTaaInfo.imageView = _temporalFilter.taaOldColorAttach.imageView;
	VkDescriptorImageInfo velocityInfo;
	velocityInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	velocityInfo.imageView = _composite.velocityAttach.imageView;
	VkDescriptorImageInfo linearSampInfo;
	linearSampInfo.sampler = _linearSampler;
	VkDescriptorImageInfo nearestSampInfo;
	nearestSampInfo.sampler = _nearestSampler;
	VkDescriptorBufferInfo settingsInfo = get_current_frame()._settingsBuffer.get_info();

	VkDescriptorSet globalSet;
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &settingsInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(1, &currentColorInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(2, &oldTaaInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(3, &velocityInfo, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(4, &linearSampInfo, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bind_image(5, &nearestSampInfo, VK_DESCRIPTOR_TYPE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(globalSet);

	if (material != nullptr)
	{
		auto shaderPass = material->original->passShaders[vkutil::MeshpassType::Forward];
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->layout, 0, 1, &globalSet, 0, nullptr);
	}
	else
	{
		LOG_FATAL("Invalid material for TAA")
	}

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_outputQuad._vertexBuffer._buffer, &offset);
	vkCmdDraw(cmd, _outputQuad._vertices.size(), 1, 0, 0);

	vkCmdEndRenderPass(cmd);

	VkImageMemoryBarrier imgBarrier1 = vkinit::image_barrier(
		_temporalFilter.taaOldColorAttach.imageData.image,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_MEMORY_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT);

	VkImageMemoryBarrier imgBarrier2 = vkinit::image_barrier(
		_temporalFilter.taaCurrentColorAttach.imageData.image,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT);

	VkImageMemoryBarrier imgBarriers[2] = { imgBarrier1, imgBarrier2 };
	
	vkCmdPipelineBarrier(cmd,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0, 0, nullptr, 0, nullptr,
		2, imgBarriers);
	
	VkExtent3D curImgExt = _temporalFilter.taaCurrentColorAttach.extent;
	VkExtent3D oldImgExt = _temporalFilter.taaOldColorAttach.extent;
	
	VkImageBlit imageBlit{};
	imageBlit.srcOffsets[1].x = static_cast<int32_t>(curImgExt.width);
	imageBlit.srcOffsets[1].y = static_cast<int32_t>(curImgExt.height);
	imageBlit.srcOffsets[1].z = 1;
	imageBlit.dstOffsets[1].x = static_cast<int32_t>(oldImgExt.width);
	imageBlit.dstOffsets[1].y = static_cast<int32_t>(oldImgExt.height);
	imageBlit.dstOffsets[1].z = 1;
	imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.srcSubresource.layerCount = 1;
	imageBlit.srcSubresource.mipLevel = 0;
	imageBlit.srcSubresource.baseArrayLayer = 0;
	imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.dstSubresource.layerCount = 1;
	imageBlit.dstSubresource.mipLevel = 0;
	imageBlit.dstSubresource.baseArrayLayer = 0;
	
	VkImage curImage = _temporalFilter.taaCurrentColorAttach.imageData.image;
	VkImage oldImage = _temporalFilter.taaOldColorAttach.imageData.image;
	vkCmdBlitImage(
		cmd,
		curImage,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		oldImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&imageBlit,
		VK_FILTER_LINEAR);
}

void VulkanEngine::draw_final_quad(VkCommandBuffer cmd, uint32_t swapchainImageIndex)
{
	VkImageMemoryBarrier imgBarrier1 = vkinit::image_barrier(
		_temporalFilter.taaOldColorAttach.imageData.image,
		VK_ACCESS_MEMORY_WRITE_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT);

	VkImageMemoryBarrier imgBarrier2 = vkinit::image_barrier(
		_temporalFilter.taaCurrentColorAttach.imageData.image,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_SHADER_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT);

	VkImageMemoryBarrier imgBarriers[2] = { imgBarrier1, imgBarrier2 };
	
	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr,
		2, imgBarriers);
	
	VkClearValue clearValue;
	clearValue.color = { { 0.15f, 0.15f, 0.15f, 1.0f } };
	
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(_renderPass, _windowExtent, _framebuffers[swapchainImageIndex]);
		
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(get_current_frame()._mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	auto material = _materialSystem.get_material("Postprocessing");
	
	VkDescriptorImageInfo taaCurrentImage;
	taaCurrentImage.sampler = _linearSampler;
	taaCurrentImage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	taaCurrentImage.imageView = _temporalFilter.taaCurrentColorAttach.imageView;
	//
	// VkDescriptorImageInfo GBufferDepthImgInfo;
	// GBufferDepthImgInfo.sampler = _linearSampler;
	// GBufferDepthImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// GBufferDepthImgInfo.imageView = _GBuffer.depth.imageView;
	//
	// VkDescriptorImageInfo transpColorImgInfo;
	// transpColorImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// transpColorImgInfo.imageView = _transparencyColorAttach.imageView;
	// transpColorImgInfo.sampler = _linearSampler;
	//
	// VkDescriptorImageInfo transpDepthImgInfo;
	// transpDepthImgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// transpDepthImgInfo.imageView = _transparencyDepthAttach.imageView;
	// transpDepthImgInfo.sampler = _linearSampler;
	//
	VkDescriptorSet offscrColorImageSet;
	
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_image(0, &taaCurrentImage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(offscrColorImageSet);
		
	if (material != nullptr)
	{
		auto meshPass = material->original->passShaders[vkutil::MeshpassType::Forward];
		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPass->pipeline);
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, meshPass->layout, 0, 1, &offscrColorImageSet, 0, nullptr);
	}
	else
	{
		LOG_FATAL("Invalid material for output quad, draw_output_quad func")
	}

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, &_outputQuad._vertexBuffer._buffer, &offset);
	vkCmdDraw(cmd, _outputQuad._vertices.size(), 1, 0, 0);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), get_current_frame()._mainCommandBuffer);
		
	vkCmdEndRenderPass(get_current_frame()._mainCommandBuffer);
}

void VulkanEngine::submit(VkCommandBuffer cmd, uint32_t swapchainImageIndex)
{
	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;
	
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submit.pWaitDstStageMask = &waitStage;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &get_current_frame()._presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &get_current_frame()._renderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &get_current_frame()._mainCommandBuffer;
	VkResult result = vkQueueSubmit(_graphicsQueue, 1, &submit, get_current_frame()._renderFence);
	if (result != VK_SUCCESS)
	{
		LOG_FATAL("Queue submit fatal error {}", result);
		return;
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;

	presentInfo.pSwapchains = &_swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &get_current_frame()._renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;
	result = vkQueuePresentKHR(_graphicsQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		refresh_swapchain();
		return;
	}
}

void VulkanEngine::bake_shadow_maps(VkCommandBuffer cmd)
{
	size_t size = sizeof(glm::mat4) * _renderScene._dirLights.size();

	std::vector<bool>& dirMaps = _renderScene._bNeedsBakeDirShadows;
	std::vector<bool>& pointMaps = _renderScene._bNeedsBakePointShadows;
	std::vector<bool>& spotMaps = _renderScene._bNeedsBakeSpotShadows;
	
	float nearPlane = 0.1f, farPlane = 85.0f;

	RenderScene* scene = &_renderScene;
	
	size_t dirLightsBufSize = sizeof(actors::DirectionLight) * scene->_dirLights.size();
	size_t pointLightsBufSize = sizeof(actors::PointLight) * scene->_pointLights.size();
	size_t spotLightsBufSize = sizeof(actors::SpotLight) * scene->_spotLights.size();
	
	std::vector<bool>& dirLights = _renderScene._bNeedsRealoadingDirLights;
	std::vector<bool>& pointLights = _renderScene._bNeedsReloadingPointLights;
	std::vector<bool>& spotLights = _renderScene._bNeedsReloadingSpotLights;
		
	if (std::find(dirMaps.begin(), dirMaps.end(), true) != dirMaps.end())
	{
		LOG_INFO("Bake dir lights");
		_renderScene._dirShadowMapsInfos.clear();
		for (int i = 0; i != _renderScene._dirLights.size(); ++i)
		{
			auto& dirLight = _renderScene._dirLights[i];
			auto& shadowMap = _renderScene._dirShadowMaps[i];

			ShadowMap::create_light_space_matrices(this, ActorType::DirectionalLight, i, shadowMap);
			size_t allocSize = sizeof(actors::DirectionLight);
			AllocatedBuffer& buffer = _renderScene._dirShadowMaps[i].lightBuffer;
			reallocate_buffer(buffer, allocSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			buffer.copy_from(this, &dirLight, allocSize);
		}

		reallocate_light_buffer(ActorType::DirectionalLight);

		draw_shadow_pass(cmd, vkutil::MeshpassType::DirectionalShadow);

		for (auto& shadowMap : _renderScene._dirShadowMaps)
		{
			VkDescriptorImageInfo info;
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.imageView = shadowMap.attachment.imageView;
			info.sampler = nullptr;

			_renderScene._dirShadowMapsInfos.push_back(info);
		}

		dirMaps = { false };
	}

	if (std::find(pointMaps.begin(), pointMaps.end(), true) != pointMaps.end())
	{
		LOG_INFO("Bake point lights");
		_renderScene._pointShadowMapsInfos.clear();
		for (int i = 0; i != _renderScene._pointLights.size(); ++i)
		{
			ShadowMap& shadowMap = _renderScene._pointShadowMaps[i];
			auto& pointLight = _renderScene._pointLights[i];

			ShadowMap::create_light_space_matrices(this, ActorType::PointLight, i, shadowMap);
			size_t allocSize = sizeof(actors::PointLight);
			AllocatedBuffer& buffer = shadowMap.lightBuffer;
			reallocate_buffer(buffer, allocSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			buffer.copy_from(this, &pointLight, allocSize);

			VkDescriptorImageInfo info;
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.imageView = shadowMap.attachment.imageView;
			info.sampler = nullptr;

			_renderScene._pointShadowMapsInfos.push_back(info);
		}

		reallocate_light_buffer(ActorType::PointLight);

		draw_shadow_pass(cmd, vkutil::MeshpassType::PointShadow);

		pointMaps = { false };
	}

	if (std::find(spotMaps.begin(), spotMaps.end(), true) != spotMaps.end())
	{
		LOG_INFO("Bake spot lights");
		_renderScene._spotShadowMapsInfos.clear();
		for (int i = 0; i != _renderScene._spotLights.size(); ++i)
		{
			ShadowMap& shadowMap = _renderScene._spotShadowMaps[i];
			auto& spotLight = _renderScene._spotLights[i];

			ShadowMap::create_light_space_matrices(this, ActorType::SpotLight, i, shadowMap);
			size_t allocSize = sizeof(actors::SpotLight);
			AllocatedBuffer& buffer = shadowMap.lightBuffer;
			reallocate_buffer(buffer, allocSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			buffer.copy_from(this, &spotLight, allocSize);

			VkDescriptorImageInfo info;
			info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			info.imageView = shadowMap.attachment.imageView;
			info.sampler = nullptr;

			_renderScene._spotShadowMapsInfos.push_back(info);
		}

		reallocate_light_buffer(ActorType::SpotLight);

		draw_shadow_pass(cmd, vkutil::MeshpassType::SpotShadow);

		spotMaps = { false };
	}
}

void VulkanEngine::draw_shadow_pass(VkCommandBuffer cmd, vkutil::MeshpassType passType)
{
	std::vector<ShadowMap> shadowMaps;
	RenderScene::MeshPass* meshPass;

	switch(passType)
	{
		case vkutil::MeshpassType::DirectionalShadow:
		{
			shadowMaps = _renderScene._dirShadowMaps;
			meshPass = &_renderScene._dirShadowPass;
			break;
		}
		case vkutil::MeshpassType::PointShadow:
		{
			shadowMaps = _renderScene._pointShadowMaps;
			meshPass = &_renderScene._pointShadowPass;
			break;
		}
		case vkutil::MeshpassType::SpotShadow:
		{
			shadowMaps = _renderScene._spotShadowMaps;
			meshPass = &_renderScene._spotShadowPass;
			break;
		}
		case vkutil::MeshpassType::Forward:
		case vkutil::MeshpassType::Transparency:
		case vkutil::MeshpassType::Deferred:
		case vkutil::MeshpassType::None:
			LOG_WARNING("This function was created for shadow passes");
			return;
	}

	vkutil::ShaderPass* prevMaterial = nullptr;
	VkDeviceSize offset = 0;

	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;
	
	VkDescriptorBufferInfo objectDataBufferInfo = _renderScene._objectDataBuffer.get_info(true);
	VkDescriptorBufferInfo finalInstanceBufferInfo = meshPass->compactedInstanceBuffer.get_info(true);

	VkDescriptorSet dirShadowsDescriptorSet;
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &objectDataBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.bind_buffer(1, &finalInstanceBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.build(dirShadowsDescriptorSet);
	
	for (int i = 0; i != shadowMaps.size(); ++i)
	{
		vkCmdBindVertexBuffers(cmd, 0, 1, &_renderScene._globalVertexBuffer._buffer, &offset);
		vkCmdBindIndexBuffer(cmd, _renderScene._globalIndexBuffer._buffer, offset, VK_INDEX_TYPE_UINT32);
		
		//actors::DirectionLight& dirLight = _renderScene._dirLights[i];
		ShadowMap& shadowMap = shadowMaps[i];

		CullParams cullParams;
		cullParams.frustumCull = true;
		cullParams.occlusionCull = false;
		cullParams.aabb = false;
		cullParams.drawDist = 9999999;
		cullParams.viewMatrix = shadowMap.lightViewMat;
		cullParams.projMatrix = shadowMap.lightProjMat;

		culling(*meshPass, cmd, cullParams);
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
			0, 0, nullptr, _afterCullingBufferBarriers.size(), _afterCullingBufferBarriers.data(), 0, nullptr);

		_afterCullingBufferBarriers.clear();

		draw_objects_in_shadow_pass(cmd, dirShadowsDescriptorSet, *meshPass, i);
		
		VkImage& image = shadowMap.attachment.imageData.image;
		VkImageMemoryBarrier imageBarrier = vkinit::image_barrier(image,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_ASPECT_DEPTH_BIT);

		_afterShadowsBarriers.push_back(imageBarrier);
	}
}

void VulkanEngine::draw_objects_in_shadow_pass(VkCommandBuffer cmd, VkDescriptorSet globalDescriptorSet, RenderScene::MeshPass& meshPass, uint32_t id)
{
	VkDescriptorBufferInfo shadowMapBufferInfo;

	ShadowMap shadowMap;
	
	switch(meshPass.type)
	{
		case vkutil::MeshpassType::DirectionalShadow:
		{
			shadowMap = _renderScene._dirShadowMaps[id];
			break;
		}
		case vkutil::MeshpassType::PointShadow:
		{
			shadowMap = _renderScene._pointShadowMaps[id];
			break;
		}
		case vkutil::MeshpassType::SpotShadow:
		{
			shadowMap = _renderScene._spotShadowMaps[id];
			break;
		}
		case vkutil::MeshpassType::Forward:
		case vkutil::MeshpassType::Transparency:
		case vkutil::MeshpassType::Deferred:
		case vkutil::MeshpassType::None:
			LOG_WARNING("This function was created only for shader passes");
			return;
	}
	
	shadowMapBufferInfo = shadowMap.lightBuffer.get_info();
	
	VkDescriptorSet lightDataDescriptorSet;
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &shadowMapBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.build(lightDataDescriptorSet);

	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	VkExtent2D extent = { shadowMap.attachment.extent.width, shadowMap.attachment.extent.height };
	VkRenderPassBeginInfo rpInfo = vkinit::renderpass_begin_info(shadowMap.renderPass, extent, shadowMap.framebuffer);
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &depthClear;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	VkRect2D scissor;
	scissor.offset = { 0, 0 };
	scissor.extent = extent;
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	vkCmdSetDepthBias(cmd, 5.0f, 0.0f, 6.5f);

	vkutil::ShaderPass* prevMaterial = nullptr;
	
	for (int i = 0; i != meshPass.multibatches.size(); ++i)
	{
		auto& multibatch = meshPass.multibatches[i];
		auto& batch = meshPass.batches[multibatch.first];

		auto shaderPass = batch.material.shaderPass;

		if (shaderPass != prevMaterial)
		{
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->pipeline);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->layout, 0, 1, &globalDescriptorSet, 0, nullptr);
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, shaderPass->layout, 1, 1, &lightDataDescriptorSet, 0, nullptr);
			prevMaterial = shaderPass;
		}

		VkDeviceSize offset = multibatch.first * sizeof(GPUIndirectObject);
		uint32_t stride = sizeof(GPUIndirectObject);

		vkCmdDrawIndexedIndirect(cmd, meshPass.drawIndirectBuffer._buffer, offset, multibatch.count, stride);
	}

	vkCmdEndRenderPass(cmd);
}

void VulkanEngine::reallocate_light_buffer(ActorType lightType)
{
	RenderScene* scene = &_renderScene;

	switch (lightType)
	{
		case (ActorType::DirectionalLight):
		{
			size_t dirLightsBufSize = sizeof(actors::DirectionLight) * scene->_dirLights.size();
			std::vector<bool>& dirLights = _renderScene._bNeedsRealoadingDirLights;
			std::vector<bool>& dirMaps = _renderScene._bNeedsBakeDirShadows;
			
			reallocate_buffer(scene->_dirLightsBuffer, dirLightsBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			AllocatedBufferT<actors::DirectionLight> tempBuffer(this, dirLightsBufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
			tempBuffer.copy_from(this, scene->_dirLights.data(), dirLightsBufSize);
			immediate_submit([&](VkCommandBuffer cmd){
				AllocatedBufferT<actors::DirectionLight>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &scene->_dirLightsBuffer);
			});
			tempBuffer.destroy_buffer(this);

			dirLights = { false };
		}
		case (ActorType::SpotLight):
		{
			size_t spotLightsBufSize = sizeof(actors::SpotLight) * scene->_spotLights.size();
			std::vector<bool>& spotLights = _renderScene._bNeedsReloadingSpotLights;
			std::vector<bool>& spotMaps = _renderScene._bNeedsBakeSpotShadows;
		
			reallocate_buffer(scene->_spotLightsBuffer, spotLightsBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			AllocatedBufferT<actors::SpotLight> tempBuffer(this, spotLightsBufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
			tempBuffer.copy_from(this, scene->_spotLights.data(), spotLightsBufSize);
			immediate_submit([&](VkCommandBuffer cmd){
				AllocatedBufferT<actors::SpotLight>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &scene->_spotLightsBuffer);
			});
			tempBuffer.destroy_buffer(this);

			spotLights = { false };
		}
		case (ActorType::PointLight):
		{
			size_t pointLightsBufSize = sizeof(actors::PointLight) * scene->_pointLights.size();
			std::vector<bool>& pointLights = _renderScene._bNeedsReloadingPointLights;
			std::vector<bool>& pointMaps = _renderScene._bNeedsBakePointShadows;
		
			reallocate_buffer(scene->_pointLightsBuffer, pointLightsBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			AllocatedBufferT<actors::PointLight> tempBuffer(this, pointLightsBufSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
			tempBuffer.copy_from(this, scene->_pointLights.data(), pointLightsBufSize);
			immediate_submit([&](VkCommandBuffer cmd){
				AllocatedBufferT<actors::PointLight>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &scene->_pointLightsBuffer);
			});
			tempBuffer.destroy_buffer(this);

			pointLights = { false };
		}
	}
}

void VulkanEngine::culling(RenderScene::MeshPass& meshPass, VkCommandBuffer cmd, CullParams cullParams)
{
	VkDescriptorBufferInfo objectDataBufferInfo = _renderScene._objectDataBuffer.get_info(true);
	VkDescriptorBufferInfo cameraBufferInfo = get_current_frame()._cameraBuffer.get_info();
	VkDescriptorBufferInfo drawIndirectBufferInfo = meshPass.drawIndirectBuffer.get_info(true);
	VkDescriptorBufferInfo gpuInstanceBufferInfo = meshPass.passObjectsBuffer.get_info(true);
	VkDescriptorBufferInfo finalInstanceBufferInfo = meshPass.compactedInstanceBuffer.get_info(true);

	VkDescriptorImageInfo depthPyramidImageInfo;
	depthPyramidImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	depthPyramidImageInfo.imageView = _depthPyramid.imageView;
	depthPyramidImageInfo.sampler = _depthSampler;

	VkDescriptorSet cullingSet;
	vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
		.bind_buffer(0, &objectDataBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_buffer(1, &drawIndirectBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_buffer(2, &gpuInstanceBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_buffer(3, &finalInstanceBufferInfo, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_image(4, &depthPyramidImageInfo, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
		.bind_buffer(5, &cameraBufferInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
		.build(cullingSet);

	glm::mat4 projection = cullParams.projMatrix;
	glm::mat4 projectionT = glm::transpose(projection);

	glm::vec4 frustumX = normalize_plane(projectionT[3] + projectionT[0]);
	glm::vec4 frustumY = normalize_plane(projectionT[3] + projectionT[1]);
		
	CullData cullData;
	cullData.P00 = projection[0][0];
	cullData.P11 = projection[1][1];
	cullData.znear = 0.001f;
	cullData.zfar = cullParams.drawDist;
	cullData.frustum[0] = frustumX.x;
	cullData.frustum[1] = frustumX.z;
	cullData.frustum[2] = frustumY.y;
	cullData.frustum[3] = frustumY.z;
	cullData.drawCount = static_cast<uint32_t>(meshPass.flatBatches.size());
	cullData.cullingEnabled = cullParams.frustumCull;
	cullData.lodEnabled = false;
	cullData.occlusionEnabled = cullParams.occlusionCull;
	cullData.lodBase = 10.0f;
	cullData.lodStep = 1.5f;
	cullData.pyramidWidth = static_cast<uint32_t>(_depthPyramidWidth);
	cullData.pyramidHeight = static_cast<uint32_t>(_depthPyramidHeight);
	cullData.view = cullParams.viewMatrix;

	cullData.AABBcheck = cullParams.aabb;
	cullData.aabbmin_x = cullParams.aabbmin.x;
	cullData.aabbmin_y = cullParams.aabbmin.y;
	cullData.aabbmin_z = cullParams.aabbmin.z;
	cullData.aabbmax_x = cullParams.aabbmax.x;
	cullData.aabbmax_y = cullParams.aabbmax.y;
	cullData.aabbmax_z = cullParams.aabbmax.z;

	if (cullParams.drawDist > 10000)
	{
		cullData.distCull = false;
	}
	else
	{
		cullData.distCull = true;
	}

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _cullingPipeline);
	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _cullintPipelineLayout, 0,1, &cullingSet, 0, nullptr);
	vkCmdPushConstants(cmd, _cullintPipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CullData), &cullData);
	vkCmdDispatch(cmd, static_cast<uint32_t>((meshPass.flatBatches.size() / 256) + 1), 1, 1);

	VkBufferMemoryBarrier barrier1 = vkinit::buffer_barrier(&meshPass.compactedInstanceBuffer, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, _graphicsQueueFamily);
	_afterCullingBufferBarriers.push_back(barrier1);

	VkBufferMemoryBarrier barrier2 = vkinit::buffer_barrier(&meshPass.drawIndirectBuffer, VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_INDIRECT_COMMAND_READ_BIT, _graphicsQueueFamily);
	_afterCullingBufferBarriers.push_back(barrier2);
}

void VulkanEngine::prepare_data_for_drawing(VkCommandBuffer cmd)
{
	RenderScene* scene = &_renderScene;

	size_t allocSize = scene->_renderables.size() * sizeof(GPUObjectData);
	if (allocSize != scene->_objectDataBuffer._bufferSize)
	{
		reallocate_buffer(scene->_objectDataBuffer, allocSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		void* data;
		vmaMapMemory(_allocator, scene->_objectDataBuffer._allocation, &data);
		scene->fill_object_data(reinterpret_cast<GPUObjectData*>(data));
		vmaUnmapMemory(_allocator, scene->_objectDataBuffer._allocation);

		scene->_dirtyObjects.clear();
	}

	if (scene->_renderables.size() * 0.8f >= scene->_dirtyObjects.size())
	{
		void* data;
		vmaMapMemory(_allocator, scene->_objectDataBuffer._allocation, &data);

		for (auto& handle : scene->_dirtyObjects)
		{
			uint32_t offset = handle.handle;
			scene->write_object(reinterpret_cast<GPUObjectData*>(data) + offset, handle);
		}

		vmaUnmapMemory(_allocator, scene->_objectDataBuffer._allocation);

		scene->_dirtyObjects.clear();
	}
	else
	{
		AllocatedBufferT<GPUObjectData> tempBuffer(this, allocSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
		void* data;
		vmaMapMemory(_allocator, tempBuffer._allocation, &data);
		scene->fill_object_data(reinterpret_cast<GPUObjectData*>(data));
		vmaUnmapMemory(_allocator, tempBuffer._allocation);
		
		get_current_frame()._frameDeletionQueue.push_function([=](){
			scene->_objectDataBuffer.destroy_buffer(this);
		});

		AllocatedBufferT<GPUObjectData>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &scene->_objectDataBuffer);
		scene->_dirtyObjects.clear();
	}

	size_t dirLightsBufSize = sizeof(actors::DirectionLight) * scene->_dirLights.size();
	size_t pointLightsBufSize = sizeof(actors::PointLight) * scene->_pointLights.size();
	size_t spotLightsBufSize = sizeof(actors::SpotLight) * scene->_spotLights.size();

	std::vector<bool>& dirLights = _renderScene._bNeedsRealoadingDirLights;
	std::vector<bool>& pointLights = _renderScene._bNeedsReloadingPointLights;
	std::vector<bool>& spotLights = _renderScene._bNeedsReloadingSpotLights;

	std::vector<bool>& dirMaps = _renderScene._bNeedsBakeDirShadows;
	std::vector<bool>& pointMaps = _renderScene._bNeedsBakePointShadows;
	std::vector<bool>& spotMaps = _renderScene._bNeedsBakeSpotShadows;

	if (std::find(dirLights.begin(), dirLights.end(), true) != dirLights.end() && std::find(dirMaps.begin(), dirMaps.end(), true) == dirMaps.end())
	{
		reallocate_light_buffer(DirectionalLight);
	}
	if (std::find(spotLights.begin(), spotLights.end(), true) != spotLights.end() && std::find(spotMaps.begin(), spotMaps.end(), true) == spotMaps.end())
	{
		reallocate_light_buffer(SpotLight);
	}
	if (std::find(pointLights.begin(), pointLights.end(), true) != pointLights.end() && std::find(pointMaps.begin(), pointMaps.end(), true) == pointMaps.end())
	{		
		reallocate_light_buffer(PointLight);
	}
	
	std::vector<RenderScene::MeshPass*> passes = {
		&_renderScene._deferredPass,
		//&_renderScene._forwardPass,
		&_renderScene._dirShadowPass,
		&_renderScene._pointShadowPass,
		&_renderScene._spotShadowPass,
		&_renderScene._transparentForwardPass
	};

	for (int i = 0; i != passes.size(); ++i)
	{
		auto& pass = *passes[i];

		size_t compactedBufSize = pass.flatBatches.size() * sizeof(uint32_t);
		size_t passObjBufSize = pass.flatBatches.size() * sizeof(GPUInstance);
		size_t indirectBufSize = pass.batches.size() * sizeof(GPUIndirectObject);

		if (pass.compactedInstanceBuffer._bufferSize != compactedBufSize)
		{
			reallocate_buffer(pass.compactedInstanceBuffer, compactedBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		}
		if (pass.passObjectsBuffer._bufferSize != passObjBufSize)
		{
			reallocate_buffer(pass.passObjectsBuffer, passObjBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		}
		if (pass.drawIndirectBuffer._bufferSize != indirectBufSize)
		{
			reallocate_buffer(pass.drawIndirectBuffer, indirectBufSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
		}
	}

	std::vector<std::future<void>> asyncCalls;
	asyncCalls.reserve(passes.size() * 3);

	std::vector<AllocatedBuffer> unmapMemory;
	std::vector<VkBufferMemoryBarrier> bufferMemoryBarriers;

	for (int i = 0; i != passes.size(); ++i)
	{
		auto& pass = *passes[i];
		auto pPass = passes[i];
		
		if (pass.batches.size() > 0 && pass.needsIndirectRefresh)
		{
			AllocatedBufferT<GPUIndirectObject> tempBuffer(this, pass.drawIndirectBuffer._bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU);

			void* data;
			vmaMapMemory(_allocator, tempBuffer._allocation, &data);

			asyncCalls.push_back(std::async(std::launch::async, [=]{
				scene->fill_indirect_array(reinterpret_cast<GPUIndirectObject*>(data), *pPass);
			}));

			if (pass.clearIndirectBuffer._buffer != VK_NULL_HANDLE)
			{
				get_current_frame()._frameDeletionQueue.push_function([=](){
					pPass->clearIndirectBuffer.destroy_buffer(this);
				});
			}

			unmapMemory.push_back(tempBuffer);

			pass.needsIndirectRefresh = false;
			pass.clearIndirectBuffer = tempBuffer;			
		}

		if (pass.flatBatches.size() > 0 && pass.needsInstanceRefresh)
		{
			AllocatedBufferT<GPUInstance> tempBuffer(this, pass.passObjectsBuffer._bufferSize,
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU);

			void* data;
			vmaMapMemory(_allocator, tempBuffer._allocation, &data);
			asyncCalls.push_back(std::async(std::launch::async, [=](){
				scene->fill_instances_array(reinterpret_cast<GPUInstance*>(data), *pPass);
			}));

			unmapMemory.push_back(tempBuffer);

			get_current_frame()._frameDeletionQueue.push_function([=](){
				vmaDestroyBuffer(_allocator, tempBuffer._buffer, tempBuffer._allocation);
			});

			AllocatedBufferT<GPUInstance>::copy_typed_buffer_cmd(this, cmd, &tempBuffer, &pass.passObjectsBuffer);

			VkBufferMemoryBarrier barrier = vkinit::buffer_barrier(&pass.passObjectsBuffer,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT,
				_graphicsQueueFamily);

			bufferMemoryBarriers.push_back(barrier);

			pass.needsInstanceRefresh = false;
		}
	}

	for (auto& call : asyncCalls)
	{
		call.get();
	}

	for (auto& unmap : unmapMemory)
	{
		vmaUnmapMemory(_allocator, unmap._allocation);
	}

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0,
		nullptr, static_cast<uint32_t>(bufferMemoryBarriers.size()), bufferMemoryBarriers.data(), 0, nullptr);

	bufferMemoryBarriers.clear();
}

void VulkanEngine::prepare_per_frame_data(VkCommandBuffer cmd)
{
	glm::mat4 view = camera.get_view_matrix();
	glm::mat4 projection = camera.get_projection_matrix((float)_windowExtent.width, (float)_windowExtent.height);
 
	GPUCameraData camData;
	camData.oldView = _temporalFilter.oldView;
	camData.view = view;
	camData.proj = projection;
	camData.viewproj = projection * view;
	camData.invViewProj = glm::inverse(camData.view) * glm::inverse(camData.proj);
	camData.cameraPosition = glm::vec4(camera.get_position(), 1.0);
	_temporalFilter.oldView = camData.view;

	get_current_frame()._cameraBuffer.copy_from(this, &camData, sizeof(GPUCameraData));
	
	GPUSceneData sceneData;
	sceneData.dirLightsAmount = _renderScene._dirLights.size();
	sceneData.pointLightsAmount = _renderScene._pointLights.size();
	sceneData.spotLightsAmount = _renderScene._spotLights.size();

	get_current_frame()._sceneDataBuffer.copy_from(this, &sceneData, sizeof(GPUSceneData));
	
	_settings.viewportRes.x = _windowExtent.width;
	_settings.viewportRes.y = _windowExtent.height;
	_settings.totalFrames = _frameNumber;

	get_current_frame()._settingsBuffer.copy_from(this, &_settings, sizeof(Settings));
}

void VulkanEngine::depth_reduce(VkCommandBuffer cmd)
{	
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _depthReducePipeline);
	
	for (int32_t i = 0; i != _depthPyramidLevels; ++i)
	{
		VkDescriptorImageInfo destTarget;
		destTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		destTarget.imageView = _depthPyramideMips[i];
		destTarget.sampler = _depthSampler;

		VkDescriptorImageInfo sourceTarget;
		sourceTarget.sampler = _depthSampler;

		if (i == 0)
		{
			sourceTarget.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			sourceTarget.imageView = _GBuffer.depth.imageView;
		}
		else
		{
			sourceTarget.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			sourceTarget.imageView = _depthPyramideMips[i - 1];
		}

		VkDescriptorSet depthSet;
		vkutil::DescriptorBuilder::begin(&_descriptorLayoutCache, &get_current_frame()._dynamicDescriptorAllocator)
			.bind_image(0, &destTarget, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.bind_image(1, &sourceTarget, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.build(depthSet);

		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, _depthReduceLayout, 0, 1, &depthSet, 0, nullptr);

		uint32_t levelWidth = _depthPyramidWidth >> i;
		uint32_t levelHeight = _depthPyramidHeight >> i;
		if (levelWidth < 1) levelWidth = 1;
		if (levelHeight < 1) levelHeight = 1;

		DepthReduceData reduceData = { glm::vec2(levelWidth, levelHeight) };
		vkCmdPushConstants(cmd, _depthReduceLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(DepthReduceData), &reduceData);
		vkCmdDispatch(cmd, get_group_count(levelWidth, 32), get_group_count(levelHeight, 32), 1);

		VkImageMemoryBarrier reduceBarrier = vkinit::image_barrier(_depthPyramid.imageData.image,
			VK_ACCESS_SHADER_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_ASPECT_COLOR_BIT);

		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &reduceBarrier);
	}

	VkImageMemoryBarrier mainDepthBarrier = vkinit::image_barrier(_GBuffer.depth.imageData.image,
		VK_ACCESS_SHADER_READ_BIT,
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT);

	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, 0, 0, 0, 1, &mainDepthBarrier);
}

void VulkanEngine::prepare_gpu_indirect_buffer(VkCommandBuffer cmd, RenderScene::MeshPass& meshPass)
{
	AllocatedBuffer::copy_buffer_cmd(this, cmd, &meshPass.clearIndirectBuffer, &meshPass.drawIndirectBuffer);
	VkBufferMemoryBarrier memoryBarrier = vkinit::buffer_barrier(&meshPass.drawIndirectBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, _graphicsQueueFamily);
	_beforeCullingBufferBarriers.push_back(memoryBarrier);
}
