﻿#include "material_submanager.h"

#include "engine_core/material/materials.h"
#include "engine_core/material/material_settings.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

constexpr uint32_t MATERIALS_INIT_NUMBER = 64;

MaterialSubmanager::MaterialSubmanager(SceneSubmanagerInitializationContext& initContext)
	: SceneSubmanagerBase(initContext)
{
	create_samplers();
}

void MaterialSubmanager::update(rhi::CommandBuffer& cmdBuffer)
{
	if (!_areGPUBuffersAllocated)
		allocate_gpu_buffers();

	update_cpu_arrays(cmdBuffer);

	_rendererResourceManager->update_buffer(
		&cmdBuffer,
		MATERIAL_BUFFER_NAME,
		sizeof(RendererMaterial),
		_rendererMaterials.data(),
		_rendererMaterials.size(),
		_rendererMaterials.size());
}

void MaterialSubmanager::cleanup_after_update()
{
	_rendererMaterials.clear();
}

bool MaterialSubmanager::need_update()
{
	return true;
}

uint32_t MaterialSubmanager::get_gpu_material_index(UUID cpuMaterialUUID)
{
	auto it = _gpuOpaqueMaterialIndexByCPUMaterialUUID.find(cpuMaterialUUID);
	if (it != _gpuOpaqueMaterialIndexByCPUMaterialUUID.end())
		return it->second;

	LOG_FATAL("MaterialSubmanager::get_gpu_material_index(): There is no gpu material by uuid {}", cpuMaterialUUID)
}

void MaterialSubmanager::subscribe_to_events()
{
	
}

void MaterialSubmanager::allocate_gpu_buffers()
{
	_rendererResourceManager->allocate_storage_buffer(MATERIAL_BUFFER_NAME, MATERIALS_INIT_NUMBER * sizeof(RendererMaterial));
	_areGPUBuffersAllocated = true;
}

void MaterialSubmanager::update_cpu_arrays(rhi::CommandBuffer& cmd)
{
	for (auto pair : _gpuOpaqueMaterialIndexByCPUMaterialUUID)
	{
		ecore::OpaquePBRMaterial* material = _resourceManager->get_resource<ecore::OpaquePBRMaterial>(pair.first).get_resource();
		ecore::OpaquePBRMaterialSettings* materialSettings = material->get_material_settings();

		uint32_t rendererMaterialIndex = _rendererMaterials.size();
		RendererMaterial& rendererMaterial = _rendererMaterials.emplace_back();
		_gpuOpaqueMaterialIndexByCPUMaterialUUID[material->get_uuid()] = rendererMaterialIndex;

		auto it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->baseColorTextureUUID);
		if (it == _gpuTextureViewByCPUTextureUUID.end())
		{
			rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->baseColorTextureUUID);
			rendererMaterial.textures[ALBEDO].textureIndex = _rhi->get_descriptor_index(view);
		}
		else
		{
			rendererMaterial.textures[ALBEDO].textureIndex = _rhi->get_descriptor_index(it->second);
		}

		it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->roughnessTextureUUID);
		if (it == _gpuTextureViewByCPUTextureUUID.end())
		{
			rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->roughnessTextureUUID);
			rendererMaterial.textures[ROUGHNESS].textureIndex = _rhi->get_descriptor_index(view);
		}
		else
		{
			rendererMaterial.textures[ROUGHNESS].textureIndex = _rhi->get_descriptor_index(it->second);
		}

		it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->normalTextureUUID);
		if (it == _gpuTextureViewByCPUTextureUUID.end())
		{
			rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->normalTextureUUID);
			rendererMaterial.textures[NORMAL].textureIndex = _rhi->get_descriptor_index(view);
		}
		else
		{
			rendererMaterial.textures[NORMAL].textureIndex = _rhi->get_descriptor_index(it->second);
		}

		it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->ambientOcclusionTextureUUID);
		if (it == _gpuTextureViewByCPUTextureUUID.end())
		{
			rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->ambientOcclusionTextureUUID);
			rendererMaterial.textures[AO].textureIndex = _rhi->get_descriptor_index(view);
		}
		else
		{
			rendererMaterial.textures[AO].textureIndex = _rhi->get_descriptor_index(it->second);
		}

		it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->metallicTextureUUID);
		if (it == _gpuTextureViewByCPUTextureUUID.end())
		{
			rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->metallicTextureUUID);
			rendererMaterial.textures[METALLIC].textureIndex = _rhi->get_descriptor_index(view);
		}
		else
		{
			rendererMaterial.textures[METALLIC].textureIndex = _rhi->get_descriptor_index(it->second);
		}

		rendererMaterial.set_sampler_index(_rhi->get_descriptor_index(&_samplers[SAMPLER_LINEAR_REPEAT]));
	}
}

rhi::TextureView* MaterialSubmanager::allocate_2d_texture(rhi::CommandBuffer& cmd, UUID uuid)
{
	auto texture = _resourceManager->get_resource<ecore::Texture2D>(uuid).get_resource();
	ecore::Texture2DGPUAllocationContext allocContext = texture->get_allocation_context();
	rhi::Texture* gpuTexture = _rendererResourceManager->allocate_custom_texture(texture->get_name()->get_full_name(), allocContext.width, allocContext.height);
	_rendererResourceManager->update_2d_texture(&cmd, texture->get_name()->get_full_name(), allocContext.data, allocContext.width, allocContext.height);
	rhi::TextureView* gpuTextureView = _rendererResourceManager->allocate_texture_view(texture->get_name()->get_full_name(), texture->get_name()->get_full_name());

	_gpuTextureViewByCPUTextureUUID[texture->get_uuid()] = gpuTextureView;
	return gpuTextureView;
}

void MaterialSubmanager::create_samplers()
{
	rhi::SamplerInfo samplerInfo;
	samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_LINEAR;
	samplerInfo.addressMode = rhi::AddressMode::REPEAT;
	samplerInfo.borderColor = rhi::BorderColor::FLOAT_TRANSPARENT_BLACK;
	samplerInfo.maxAnisotropy = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = std::numeric_limits<float>::max();
	_rhi->create_sampler(&_samplers[SAMPLER_LINEAR_REPEAT], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
	_rhi->create_sampler(&_samplers[SAMPLER_LINEAR_CLAMP], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
	_rhi->create_sampler(&_samplers[SAMPLER_LINEAR_MIRROR], &samplerInfo);

	samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_NEAREST;
	samplerInfo.addressMode = rhi::AddressMode::REPEAT;
	_rhi->create_sampler(&_samplers[SAMPLER_NEAREST_CLAMP], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
	_rhi->create_sampler(&_samplers[SAMPLER_NEAREST_CLAMP], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
	_rhi->create_sampler(&_samplers[SAMPLER_NEAREST_MIRROR], &samplerInfo);

}
