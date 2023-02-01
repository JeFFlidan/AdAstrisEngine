#include "material_system.h"
#include "logger.h"
#include "material_asset.h"
#include "vk_engine.h"
#include "vk_initializers.h"
#include "vk_mesh.h"
#include "vk_types.h"

#include <iostream>
#include <limits>
#include <unordered_map>
#include <functional>
#include <vulkan/vulkan_core.h>

namespace vkutil
{
	VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
	{
		_vertexInputInfo = vkinit::vertex_input_state_create_info();
		_vertexInputInfo.vertexBindingDescriptionCount = _vertexDescription.bindings.size();
		_vertexInputInfo.pVertexBindingDescriptions = _vertexDescription.bindings.data();
		_vertexInputInfo.vertexAttributeDescriptionCount = _vertexDescription.attributes.size();
		_vertexInputInfo.pVertexAttributeDescriptions = _vertexDescription.attributes.data();

		//VkDynamicState* dynamicStates = _dynamicStates;
		std::vector<VkDynamicState> dynamicStates;
		dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
		if (_rasterizer.depthBiasEnable == VK_TRUE)
			dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		_dynamicState = vkinit::dynamic_state_create_info(dynamicStates.data(), dynamicStates.size());
	
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;

		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		if (_colorBlendManyAttachments.empty())
		{
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &_colorBlendAttachment;
		}
		else
		{
			colorBlending.attachmentCount = _colorBlendManyAttachments.size();
			colorBlending.pAttachments = _colorBlendManyAttachments.data();
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;

		pipelineInfo.stageCount = _shaderStages.size();
		pipelineInfo.pStages = _shaderStages.data();
		pipelineInfo.pVertexInputState = &_vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &_inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &_rasterizer;
		pipelineInfo.pMultisampleState = &_multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = _pipelineLayout;
		pipelineInfo.renderPass = pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.pDepthStencilState = &_depthStencil;
		pipelineInfo.pDynamicState = &_dynamicState;

		VkPipeline newPipeline;

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
		{
			return VK_NULL_HANDLE;
		}
		else
		{
			return newPipeline;
		}
	}

	VkPipeline ComputePipelineBuilder::build_pipeline(VkDevice device)
	{
		VkComputePipelineCreateInfo computePipelineInfo{};
		computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineInfo.pNext = nullptr;
		computePipelineInfo.layout = _layout;
		computePipelineInfo.stage = _shaderStage;

		VkPipeline newPipeline;
		if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computePipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
		{
			LOG_FATAL("Failed to create compute pipeline");
			return VK_NULL_HANDLE;
		}

		return newPipeline;
	}

	void MaterialSystem::init(VulkanEngine* engine)
	{
		this->_engine = engine;
	}

	void MaterialSystem::setup_pipeline_builders()
	{
		vkutil::PipelineBuilder pipelineBuilder; 

		pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();
		pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		
		pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
		pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT);
		pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();
		pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

		pipelineBuilder._vertexDescription = Mesh::get_vertex_description();

		// I have to think about one pipeline builder for passes below
	    _offscrPipelineBuilder = pipelineBuilder;

	    pipelineBuilder._colorBlendManyAttachments = {
			vkinit::color_blend_attachment_state(),
			vkinit::color_blend_attachment_state(),
			vkinit::color_blend_attachment_state()
	    };
	    LOG_INFO("Blending info count {}", pipelineBuilder._colorBlendManyAttachments.size());
		_GBufferPipelineBuilder = pipelineBuilder;

		pipelineBuilder._colorBlendManyAttachments.clear();
	    
	    pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
	    pipelineBuilder._rasterizer.depthBiasEnable = VK_TRUE;
	    _dirShadowPipelineBuilder = pipelineBuilder;
	    _pointShadowPipelineBuilder = pipelineBuilder;
	    pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT);
	    pipelineBuilder._rasterizer.depthBiasEnable = VK_TRUE;
	    _spotShadowPipelineBuilder = pipelineBuilder;
	    
		pipelineBuilder._rasterizer.depthBiasEnable = VK_FALSE;
	    pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE);
	    _transparencyBuilder = pipelineBuilder;

		pipelineBuilder._vertexDescription = Plane::get_vertex_description();
		
		pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(false, false, VK_COMPARE_OP_LESS_OR_EQUAL);
		
		pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
	    _postprocessingPipelineBuilder = pipelineBuilder;
	    _deferredPipelineBuilder = pipelineBuilder;
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

	void setup_shader_stages(PipelineBuilder& builder, std::vector<ShaderEffect::ShaderStage>& stages)
	{
		for (auto& stage : stages)
		{
			builder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(stage.stage, stage.shader->get_shader_module()));
		}
	}
	
	ShaderPass* MaterialSystem::build_shader_pass(VkRenderPass& renderPass, PipelineBuilder& builder, ShaderEffect* effect)
	{
		ShaderPass* shaderPass = new ShaderPass();

		shaderPass->effect = effect;

		VkPipelineLayout layout = effect->get_pipeline_layout(_engine->_device);
		
		builder._pipelineLayout = layout;
		setup_shader_stages(builder, effect->stages);
		
		VkPipeline pipeline = builder.build_pipeline(_engine->_device, renderPass);
		shaderPass->pipeline = pipeline;
		shaderPass->layout = layout;
		shaderPass->renderPass = renderPass;
		shaderPass->pipelineBuilder = &builder;

	    builder._shaderStages.clear();

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
			
		//ShaderEffect* coloredLitEffect = build_shader_effect({
		//	"/shaders/mesh.vert.spv",
		//	"/shaders/default_lit.frag.spv"});

		ShadowMap& shadowMap = _engine->_renderScene._dirShadowMaps[0];
		ShadowMap& pointShadowMap = _engine->_renderScene._pointShadowMaps[0];
		ShadowMap& spotShadowMap = _engine->_renderScene._spotShadowMaps[0];

		//ShaderPass* texturedLitPass = build_shader_pass(_engine->_mainOpaqueRenderPass, _offscrPipelineBuilder, texturedLitEffect);
		ShaderPass* postrpocessingPass = build_shader_pass(_engine->_renderPass, _postprocessingPipelineBuilder, postprocessingEffect);
		ShaderPass* dirShadowPass = build_shader_pass(shadowMap.renderPass, _dirShadowPipelineBuilder, dirShadowEffect);
		ShaderPass* pointShadowPass = build_shader_pass(pointShadowMap.renderPass, _pointShadowPipelineBuilder, pointShadowEffect);
		ShaderPass* spotShadowPass = build_shader_pass(spotShadowMap.renderPass, _spotShadowPipelineBuilder, spotShadowEffect);
		ShaderPass* transparencyPass = build_shader_pass(_engine->_transparencyRenderPass, _transparencyBuilder, transparencyEffect);
		ShaderPass* GBufferPass = build_shader_pass(_engine->_GBuffer.renderPass, _GBufferPipelineBuilder, GBufferEffect);
		ShaderPass* deferredPass = build_shader_pass(_engine->_deferredRenderPass, _deferredPipelineBuilder, deferredEffect);
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
	}

	void MaterialSystem::refresh_default_templates()
	{
		for (auto& temp : _templateCache)
		{
			EffectTemplate* effectTemplate = &temp.second;
			PerPassData<ShaderPass*> shaderPasses = effectTemplate->passShaders;
			
			if (shaderPasses[MeshpassType::DirectionalShadow] != nullptr)
			{
				ShaderPass* shaderPass = shaderPasses[MeshpassType::DirectionalShadow];
				refresh_shader_pass(shaderPass);
			}

			if (shaderPasses[MeshpassType::Forward] != nullptr)
			{
				ShaderPass* shaderPass = shaderPasses[MeshpassType::Forward];
				refresh_shader_pass(shaderPass);
			}

			if (shaderPasses[MeshpassType::Transparency] != nullptr)
			{
				ShaderPass* shaderPass = shaderPasses[MeshpassType::Transparency];
				refresh_shader_pass(shaderPass);
			}
		}
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
			
			auto pass = templ.second.passShaders[vkutil::MeshpassType::Forward];
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

				vkutil::ShaderPass* relPass = pass->relatedShaderPasses[0];
				vkDestroyPipeline(device, relPass->pipeline, nullptr);
				vkDestroyPipelineLayout(device, relPass->layout, nullptr);
				relPass->effect->destroy_shader_modules();
			}
		}
	}

	void MaterialSystem::refresh_shader_pass(ShaderPass* shaderPass)
	{
		PipelineBuilder pipelineBuilder = *shaderPass->pipelineBuilder;
		LOG_INFO("Shader stages from effect size {}", shaderPass->effect->stages.size());
		LOG_INFO("Shader stages from builder size {}", shaderPass->pipelineBuilder->_shaderStages.size());
		vkDestroyPipeline(_engine->_device, shaderPass->pipeline, nullptr);
		pipelineBuilder._viewport.x = 0.0f;
		pipelineBuilder._viewport.y = 0.0f;
		pipelineBuilder._viewport.width = static_cast<float>(_engine->_windowExtent.width);
		pipelineBuilder._viewport.height = static_cast<float>(_engine->_windowExtent.height);
		pipelineBuilder._scissor.extent = _engine->_windowExtent;
		setup_shader_stages(pipelineBuilder, shaderPass->effect->stages);
		
		shaderPass->pipeline = pipelineBuilder.build_pipeline(_engine->_device, shaderPass->renderPass);
	}

	bool MaterialData::operator==(const MaterialData& other) const
	{
		if (baseTemplate != other.baseTemplate)
			return false;
			
		if (textures.size() != other.textures.size())
		{
			return false;
		}
		else
		{
			for (int i = 0; i != textures.size(); ++i)
			{
				if (textures[i].sampler != other.textures[i].sampler)
					return false;
				if (textures[i].imageView != other.textures[i].imageView)
					return false;
			}
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
