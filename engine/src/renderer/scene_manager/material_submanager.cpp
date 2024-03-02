#include "material_submanager.h"
#include "resource_manager/resource_events.h"
#include "engine_core/material/materials.h"
#include "engine_core/material/material_settings.h"
#include "engine_core/basic_components.h"
#include "core/global_objects.h"

using namespace ad_astris;
using namespace renderer;
using namespace impl;

constexpr uint32_t MATERIALS_INIT_NUMBER = 64;

MaterialSubmanager::MaterialSubmanager()
{
	create_samplers();
	allocate_buffers();
}

void MaterialSubmanager::update(rhi::CommandBuffer& cmdBuffer)
{
	update_cpu_arrays(cmdBuffer);

	if (_rendererMaterials->is_gpu_collection())
	{
		RENDERER_RESOURCE_MANAGER()->update_buffer(
			&cmdBuffer,
			_rendererMaterials->get_mapped_buffer(),
			_rendererMaterials->get_gpu_buffer(),
			sizeof(RendererMaterial),
			_rendererMaterials->get_element_count(),
			_rendererMaterials->get_element_count());
	}
}

void MaterialSubmanager::cleanup_after_update()
{
	_rendererMaterials->clear();
}

bool MaterialSubmanager::need_update()
{
	return true;
}

void MaterialSubmanager::add_material(ecs::Entity entity)
{
	if (entity.has_component<ecore::OpaquePBRMaterialComponent>())
	{
		UUID materialUUID = entity.get_component<ecore::OpaquePBRMaterialComponent>()->materialUUID;
		_gpuOpaqueMaterialIndexByCPUMaterialUUID.insert({ materialUUID, 0 });
	}
}

uint32_t MaterialSubmanager::get_gpu_material_index(rhi::CommandBuffer& cmd, UUID cpuMaterialUUID)
{
	auto it = _gpuOpaqueMaterialIndexByCPUMaterialUUID.find(cpuMaterialUUID);
	if (it != _gpuOpaqueMaterialIndexByCPUMaterialUUID.end())
		return it->second;
	create_gpu_material(cmd, cpuMaterialUUID);
	//LOG_FATAL("MaterialSubmanager::get_gpu_material_index(): There is no gpu material by uuid {}", cpuMaterialUUID)
}

void MaterialSubmanager::subscribe_to_events()
{
	events::EventDelegate<resource::OpaquePBRMaterialCreatedEvent> delegate1 = [&](resource::OpaquePBRMaterialCreatedEvent& event)
	{
		_gpuOpaqueMaterialIndexByCPUMaterialUUID.insert({ event.get_material_handle().get_resource()->get_uuid(), 0 });
	};
	EVENT_MANAGER()->subscribe(delegate1);
}

void MaterialSubmanager::allocate_buffers()
{
	if (RHI()->has_capability(rhi::GpuCapability::CACHE_COHERENT_UMA))
	{
		_rendererMaterials = std::make_unique<RendererResourceCollection<RendererMaterial>>(
		   MATERIAL_BUFFER_NAME,
		   MATERIALS_INIT_NUMBER * sizeof(RendererMaterial));
	}
	else
	{
		_rendererMaterials = std::make_unique<RendererResourceCollection<RendererMaterial>>(
			"Cpu" + MATERIAL_BUFFER_NAME,
			MATERIAL_BUFFER_NAME,
			rhi::ResourceUsage::STORAGE_BUFFER,
			MATERIALS_INIT_NUMBER * sizeof(RendererMaterial));
	}
}

void MaterialSubmanager::update_cpu_arrays(rhi::CommandBuffer& cmd)
{
	for (auto pair : _gpuOpaqueMaterialIndexByCPUMaterialUUID)
	{
		create_gpu_material(cmd, pair.first);
	}
}

rhi::TextureView* MaterialSubmanager::allocate_2d_texture(rhi::CommandBuffer& cmd, UUID uuid)
{
	auto texture = RESOURCE_MANAGER()->get_resource<ecore::Texture2D>(uuid).get_resource();
	ecore::Texture2DGPUAllocationContext allocContext = texture->get_allocation_context();
	rhi::Texture* gpuTexture = RENDERER_RESOURCE_MANAGER()->allocate_custom_texture(
		texture->get_name()->get_full_name(),
		allocContext.width,
		allocContext.height,
		rhi::ResourceFlags::UNDEFINED,
		math::get_mip_levels(allocContext.width, allocContext.height));
	RENDERER_RESOURCE_MANAGER()->update_2d_texture(
		&cmd,
		texture->get_name()->get_full_name(),
		allocContext.data,
		allocContext.width,
		allocContext.height);
	RENDERER_RESOURCE_MANAGER()->generate_mipmaps(&cmd, gpuTexture);
	rhi::TextureView* gpuTextureView = RENDERER_RESOURCE_MANAGER()->allocate_texture_view(
		texture->get_name()->get_full_name(),
		texture->get_name()->get_full_name());

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
	RHI()->create_sampler(&_samplers[SAMPLER_LINEAR_REPEAT], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
	RHI()->create_sampler(&_samplers[SAMPLER_LINEAR_CLAMP], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
	RHI()->create_sampler(&_samplers[SAMPLER_LINEAR_MIRROR], &samplerInfo);

	samplerInfo.filter = rhi::Filter::MIN_MAG_MIP_NEAREST;
	samplerInfo.addressMode = rhi::AddressMode::REPEAT;
	RHI()->create_sampler(&_samplers[SAMPLER_NEAREST_CLAMP], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::CLAMP_TO_EDGE;
	RHI()->create_sampler(&_samplers[SAMPLER_NEAREST_CLAMP], &samplerInfo);

	samplerInfo.addressMode = rhi::AddressMode::MIRRORED_REPEAT;
	RHI()->create_sampler(&_samplers[SAMPLER_NEAREST_MIRROR], &samplerInfo);

}

void MaterialSubmanager::create_gpu_material(rhi::CommandBuffer& cmd, UUID cpuMaterialUUID)
{
	ecore::OpaquePBRMaterial* material = RESOURCE_MANAGER()->get_resource<ecore::OpaquePBRMaterial>(cpuMaterialUUID).get_resource();

	ecore::OpaquePBRMaterialSettings* materialSettings = material->get_material_settings();

	uint32_t rendererMaterialIndex = _rendererMaterials->get_element_count();
	RendererMaterial& rendererMaterial = *_rendererMaterials->push_back();
	rendererMaterial = RendererMaterial();	// temp solution, must reset RendererMaterial every frame
	_gpuOpaqueMaterialIndexByCPUMaterialUUID[material->get_uuid()] = rendererMaterialIndex;

	auto it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->baseColorTextureUUID);
	if (it == _gpuTextureViewByCPUTextureUUID.end())
	{
		rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->baseColorTextureUUID);
		rendererMaterial.textures[ALBEDO].textureIndex = RHI()->get_descriptor_index(view);
	}
	else
	{
		rendererMaterial.textures[ALBEDO].textureIndex = RHI()->get_descriptor_index(it->second);
	}

	it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->roughnessTextureUUID);
	if (it == _gpuTextureViewByCPUTextureUUID.end())
	{
		rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->roughnessTextureUUID);
		rendererMaterial.textures[ROUGHNESS].textureIndex = RHI()->get_descriptor_index(view);
	}
	else
	{
		rendererMaterial.textures[ROUGHNESS].textureIndex = RHI()->get_descriptor_index(it->second);
	}

	it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->normalTextureUUID);
	if (it == _gpuTextureViewByCPUTextureUUID.end())
	{
		rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->normalTextureUUID);
		rendererMaterial.textures[NORMAL].textureIndex = RHI()->get_descriptor_index(view);
	}
	else
	{
		rendererMaterial.textures[NORMAL].textureIndex = RHI()->get_descriptor_index(it->second);
	}

	it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->ambientOcclusionTextureUUID);
	if (it == _gpuTextureViewByCPUTextureUUID.end())
	{
		rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->ambientOcclusionTextureUUID);
		rendererMaterial.textures[AO].textureIndex = RHI()->get_descriptor_index(view);
	}
	else
	{
		rendererMaterial.textures[AO].textureIndex = RHI()->get_descriptor_index(it->second);
	}

	it = _gpuTextureViewByCPUTextureUUID.find(materialSettings->metallicTextureUUID);
	if (it == _gpuTextureViewByCPUTextureUUID.end())
	{
		rhi::TextureView* view = allocate_2d_texture(cmd, materialSettings->metallicTextureUUID);
		rendererMaterial.textures[METALLIC].textureIndex = RHI()->get_descriptor_index(view);
	}
	else
	{
		rendererMaterial.textures[METALLIC].textureIndex = RHI()->get_descriptor_index(it->second);
	}

	rendererMaterial.set_sampler_index(RHI()->get_descriptor_index(&_samplers[SAMPLER_LINEAR_REPEAT]));
}
