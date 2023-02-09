#include "material_system.h"
#include "material_asset.h"
#include "vulkan_renderer/vk_renderer.h"
#include "vulkan_renderer/vk_initializers.h"
#include "vulkan_renderer/vk_types.h"

#include <iostream>
#include <unordered_map>
#include <functional>
#include <vulkan/vulkan_core.h>

namespace engine
{
	void MaterialSystem::init(VkRenderer* engine)
	{
		this->_engine = engine;
	}

	void MaterialSystem::setup_pipeline_builders()
	{
		GraphicsPipelineBuilder pipelineBuilder(_engine->_device);
		auto description = Mesh::get_vertex_description();
		pipelineBuilder.setup_vertex_input_state(description);
		pipelineBuilder.setup_assembly_state(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineBuilder.setup_dynamic_viewport_state();
		pipelineBuilder.setup_rasterization_state(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_COUNTER_CLOCKWISE,
			VK_TRUE);
		pipelineBuilder.setup_color_blend_state_default();
		pipelineBuilder.setup_multisample_state();
		pipelineBuilder.setup_depth_state(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
		pipelineBuilder.setup_dynamic_state(true, true);

		_dirShadowPipelineBuilder = pipelineBuilder;
		_pointShadowPipelineBuilder = pipelineBuilder;
		_spotShadowPipelineBuilder = pipelineBuilder;

		pipelineBuilder.setup_rasterization_state(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT);
		pipelineBuilder.setup_dynamic_state(true, false);
		pipelineBuilder.setup_color_blend_state_default(4);

		_GBufferPipelineBuilder = pipelineBuilder;

		pipelineBuilder.setup_rasterization_state(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE);
		pipelineBuilder.setup_color_blend_state_default(2);
		_transparencyBuilder = pipelineBuilder;

		pipelineBuilder.setup_depth_state(false, false, VK_COMPARE_OP_LESS_OR_EQUAL);
		pipelineBuilder.setup_color_blend_state_default();
		description = Plane::get_vertex_description();
		pipelineBuilder.setup_vertex_input_state(description);
		_postprocessingPipelineBuilder = pipelineBuilder;
		_deferredPipelineBuilder = pipelineBuilder;
		pipelineBuilder.setup_color_blend_state_default(2);
		_compositePipelineBuilder = pipelineBuilder;
	}

	ShaderEffect* MaterialSystem::build_shader_effect(const std::vector<std::string>& shaderPaths)
	{
		// Main order is: 0 - vertex shader, 1 - fragment shader, 2 - geometry shader ...
		ShaderEffect* shaderEffect = new ShaderEffect();

		for (int i = 0; i != shaderPaths.size(); ++i)
		{
			if (shaderPaths[i].empty())
			{
				LOG_WARNING("Shader path is empty!");
				continue;
			}

			VkShaderStageFlagBits stage;
			
			switch (i)
			{
				case 0:
					stage = VK_SHADER_STAGE_VERTEX_BIT;
					break;
				case 1:
					stage = VK_SHADER_STAGE_FRAGMENT_BIT;
					break;
			}

			auto it = _shaderCache.find(shaderPaths[i]);
			if (it != _shaderCache.end())
			{
				shaderEffect->add_stage(it->second, stage);
			}
			else
			{
				Shader* temp = new Shader{_engine->_device};
				temp->load_shader_module((_engine->_projectPath + shaderPaths[i]).c_str());
				_shaderCache[shaderPaths[i]] = temp;
				shaderEffect->add_stage(temp, stage);
			}
		}

		return shaderEffect;
	}

	ShaderPass* MaterialSystem::build_shader_pass(VkRenderPass& renderPass, GraphicsPipelineBuilder& builder, ShaderEffect* effect)
	{
		ShaderPass* shaderPass = new ShaderPass();

		shaderPass->effect = effect;

		VkPipelineLayout layout = effect->get_pipeline_layout(_engine->_device);
		
		builder.setup_shader_stages(effect->stages);
		VkPipeline pipeline = builder.build(renderPass, layout);
		shaderPass->pipeline = pipeline;
		shaderPass->layout = layout;
		shaderPass->renderPass = renderPass;
		shaderPass->pipelineBuilder = &builder;

		return shaderPass;
	}

	void MaterialSystem::build_default_templates()
	{
		setup_pipeline_builders();
		
		//ShaderEffect* texturedLitEffect = build_shader_effect({
		//	"/shaders/instancing.vert.spv",
		//	"/shaders/textured_lit.frag.spv" });
		ShaderEffect* postprocessingEffect = build_shader_effect({
		    "/shaders/postprocessing.vert.spv",
			"/shaders/postprocessing.frag.spv"});
		ShaderEffect* dirShadowEffect = build_shader_effect({
			"/shaders/dir_light_depth_map.vert.spv",
			"/shaders/dir_light_depth_map.frag.spv"
		});
		ShaderEffect* pointShadowEffect = build_shader_effect({
			"/shaders/point_light_depth_map.vert.spv",
			"/shaders/point_light_depth_map.frag.spv"
		});
		ShaderEffect* spotShadowEffect = build_shader_effect({
			"/shaders/spot_light_depth_map.vert.spv",
			"/shaders/spot_light_depth_map.frag.spv"
		});
		ShaderEffect* transparencyEffect = build_shader_effect({
			"/shaders/transparency.vert.spv",
			"/shaders/transparency.frag.spv"
		});
		ShaderEffect* GBufferEffect = build_shader_effect({
			"/shaders/GBuffer.vert.spv",
			"/shaders/GBuffer.frag.spv"
		});
		ShaderEffect* deferredEffect = build_shader_effect({
			"/shaders/deferred_lighting.vert.spv",
			"/shaders/deferred_lighting.frag.spv"
		});
		ShaderEffect* taaEffect = build_shader_effect({
			"/shaders/postprocessing.vert.spv",
			"/shaders/taa.frag.spv"
		});
		ShaderEffect* compositeEffect = build_shader_effect({
			"/shaders/postprocessing.vert.spv",
			"/shaders/composite.frag.spv"
		});
			
		//ShaderEffect* coloredLitEffect = build_shader_effect({
		//	"/shaders/mesh.vert.spv",
		//	"/shaders/default_lit.frag.spv"});

		ShadowMap& shadowMap = _engine->_renderScene._dirShadowMaps[0];
		ShadowMap& pointShadowMap = _engine->_renderScene._pointShadowMaps[0];
		ShadowMap& spotShadowMap = _engine->_renderScene._spotShadowMaps[0];

		VkRenderPass taaRenderPass = _engine->_temporalFilter.taaRenderPass;
		VkRenderPass compRenderPass = _engine->_composite.renderPass;

		//ShaderPass* texturedLitPass = build_shader_pass(_engine->_mainOpaqueRenderPass, _offscrPipelineBuilder, texturedLitEffect);
		ShaderPass* postrpocessingPass = build_shader_pass(_engine->_renderPass, _postprocessingPipelineBuilder, postprocessingEffect);
		ShaderPass* dirShadowPass = build_shader_pass(shadowMap.renderPass, _dirShadowPipelineBuilder, dirShadowEffect);
		ShaderPass* pointShadowPass = build_shader_pass(pointShadowMap.renderPass, _pointShadowPipelineBuilder, pointShadowEffect);
		ShaderPass* spotShadowPass = build_shader_pass(spotShadowMap.renderPass, _spotShadowPipelineBuilder, spotShadowEffect);
		ShaderPass* transparencyPass = build_shader_pass(_engine->_transparencyRenderPass, _transparencyBuilder, transparencyEffect);
		ShaderPass* GBufferPass = build_shader_pass(_engine->_GBuffer.renderPass, _GBufferPipelineBuilder, GBufferEffect);
		ShaderPass* deferredPass = build_shader_pass(_engine->_deferredRenderPass, _deferredPipelineBuilder, deferredEffect);
		ShaderPass* taaPass = build_shader_pass(taaRenderPass, _postprocessingPipelineBuilder, taaEffect);
		ShaderPass* compositePass = build_shader_pass(compRenderPass, _compositePipelineBuilder, compositeEffect);
		//ShaderPass* coloredLitPass = build_shader_pass(_engine->_mainOpaqueRenderPass, _offscrPipelineBuilder, coloredLitEffect);

		deferredPass->relatedShaderPasses.push_back(GBufferPass);

		EffectTemplate effectTemplate;
		effectTemplate.passShaders[MeshpassType::Deferred] = deferredPass;
		effectTemplate.passShaders[MeshpassType::DirectionalShadow] = dirShadowPass;
		effectTemplate.passShaders[MeshpassType::Transparency] = nullptr;
		effectTemplate.passShaders[MeshpassType::Forward] = nullptr;
		effectTemplate.passShaders[MeshpassType::PointShadow] = pointShadowPass;
		effectTemplate.passShaders[MeshpassType::SpotShadow] = spotShadowPass;
		effectTemplate.defaultParameters = nullptr;
		effectTemplate.transparency = assets::MaterialMode::OPAQUE;
		_templateCache["PBR_opaque"] = effectTemplate;

		effectTemplate.passShaders[MeshpassType::Deferred] = nullptr;
		effectTemplate.passShaders[MeshpassType::DirectionalShadow] = nullptr;
		effectTemplate.passShaders[MeshpassType::Transparency] = transparencyPass;
		effectTemplate.passShaders[MeshpassType::Forward] = nullptr;
		effectTemplate.passShaders[MeshpassType::PointShadow] = nullptr;
		effectTemplate.passShaders[MeshpassType::SpotShadow] = nullptr;
		_templateCache["PBR_Transparency"] = effectTemplate;

		effectTemplate.passShaders[MeshpassType::Deferred] = nullptr;
		effectTemplate.passShaders[MeshpassType::Forward] = postrpocessingPass;
		effectTemplate.passShaders[MeshpassType::DirectionalShadow] = nullptr;
		effectTemplate.passShaders[MeshpassType::PointShadow] = nullptr;
		effectTemplate.passShaders[MeshpassType::SpotShadow] = nullptr;
		effectTemplate.passShaders[MeshpassType::Transparency] = nullptr;
		_templateCache["Postprocessing"] = effectTemplate;

		effectTemplate.passShaders[MeshpassType::Forward] = taaPass;
		_templateCache["TAA"] = effectTemplate;

		effectTemplate.passShaders[MeshpassType::Forward] = compositePass;
		_templateCache["Composite"] = effectTemplate;
	}

	Material* MaterialSystem::build_material(const std::string& materialName, const MaterialData& info)
	{
		Material* mat;

		auto it = _materialCache.find(info);
		if (it != _materialCache.end())
		{
			mat = (*it).second;
			_materials[materialName] = mat;
		}
		else
		{
			Material* newMat = new Material();
			newMat->original = &_templateCache[info.baseTemplate];
			newMat->parameters = info.parameters;
			//newMat->passSets[MeshpassType::DirectionalShadow] = VK_NULL_HANDLE;
			if (!info.textures.empty())
				newMat->textures = info.textures;

			LOG_INFO("Built new material {}", materialName);
			_materialCache[info] = newMat;
			mat = newMat;
			_materials[materialName] = mat;
 		}

		return mat;
	}

	Material* MaterialSystem::get_material(const std::string& materialName)
	{
		return _materials[materialName];
	}

	void MaterialSystem::cleanup()
	{
		for (auto templ : _templateCache)
		{
			VkDevice device = _engine->_device;
			
			auto pass = templ.second.passShaders[engine::MeshpassType::Forward];
			if (pass != nullptr)
			{
				vkDestroyPipeline(device, pass->pipeline, nullptr);	
				vkDestroyPipelineLayout(device, pass->layout, nullptr);
				pass->effect->destroy_shader_modules();
			}

			pass = templ.second.passShaders[MeshpassType::DirectionalShadow];
			if (pass != nullptr)
			{
				vkDestroyPipeline(device, pass->pipeline, nullptr);
				vkDestroyPipelineLayout(device, pass->layout, nullptr);
				pass->effect->destroy_shader_modules();
			}

			pass = templ.second.passShaders[MeshpassType::PointShadow];
			if (pass != nullptr)
			{
				vkDestroyPipeline(device, pass->pipeline, nullptr);
				vkDestroyPipelineLayout(device, pass->layout, nullptr);
				pass->effect->destroy_shader_modules();
			}

			pass = templ.second.passShaders[MeshpassType::SpotShadow];
			if (pass != nullptr)
			{
				vkDestroyPipeline(device, pass->pipeline, nullptr);
				vkDestroyPipelineLayout(device, pass->layout, nullptr);
				pass->effect->destroy_shader_modules();
			}

			pass = templ.second.passShaders[MeshpassType::Transparency];
			if (pass != nullptr)
			{
				vkDestroyPipeline(device, pass->pipeline, nullptr);
				vkDestroyPipelineLayout(device, pass->layout, nullptr);
				pass->effect->destroy_shader_modules();
			}

			pass = templ.second.passShaders[MeshpassType::Deferred];
			if (pass != nullptr)
			{
				vkDestroyPipeline(device, pass->pipeline, nullptr);
				vkDestroyPipelineLayout(device, pass->layout, nullptr);
				pass->effect->destroy_shader_modules();

				engine::ShaderPass* relPass = pass->relatedShaderPasses[0];
				vkDestroyPipeline(device, relPass->pipeline, nullptr);
				vkDestroyPipelineLayout(device, relPass->layout, nullptr);
				relPass->effect->destroy_shader_modules();
			}
		}
	}

	bool MaterialData::operator==(const MaterialData& other) const
	{
		if (baseTemplate != other.baseTemplate)
			return false;
			
		if (textures.size() != other.textures.size())
		{
			return false;
		}
		for (int i = 0; i != textures.size(); ++i)
		{
			if (textures[i].sampler != other.textures[i].sampler)
				return false;
			if (textures[i].imageView != other.textures[i].imageView)
				return false;
		}
		
		if (parameters.demo != other.parameters.demo)
			return false;
	
		return true;
	}

	size_t MaterialData::hash() const
	{
		using std::size_t;
		using std::hash;

		size_t result = hash<std::string>()(baseTemplate);

		for (auto& tex : textures)
		{
			size_t texture_hash = (std::hash<size_t>()((size_t)tex.sampler) << 3) && (std::hash<size_t>()((size_t)tex.imageView) >> 7);

			result ^= std::hash<size_t>()(texture_hash);
		}

		return result;
	}
}
