#include "material_system.h"
#include "logger.h"
#include "material_asset.h"
#include "vk_engine.h"
#include "vk_initializers.h"

#include <unordered_map>
#include <functional>

namespace vkutil
{
	VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
	{
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.pNext = nullptr;

		viewportState.viewportCount = 1;
		viewportState.pViewports = &_viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &_scissor;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.pNext = nullptr;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &_colorBlendAttachment;

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

	void MaterialSystem::init(VulkanEngine* engine)
	{
		this->_engine = engine;
	}

	void MaterialSystem::setup_pipeline_builders()
	{
		vkutil::PipelineBuilder pipelineBuilder; 

		pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();
		pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		
		pipelineBuilder._viewport.x = 0.0f;
		pipelineBuilder._viewport.y = 0.0f;
		pipelineBuilder._viewport.width = static_cast<float>(_engine->_windowExtent.width);
		pipelineBuilder._viewport.height = static_cast<float>(_engine->_windowExtent.height);
		pipelineBuilder._viewport.minDepth = 0.0f;
		pipelineBuilder._viewport.maxDepth = 1.0f;

		pipelineBuilder._scissor.offset = { 0, 0 };
		pipelineBuilder._scissor.extent = _engine->_windowExtent;

		pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
		pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
		pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();
		pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();

		VertexInputDescription vertexDescription = Mesh::get_vertex_description();

		pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = vertexDescription.attributes.data();
		pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = vertexDescription.attributes.size();

		pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = vertexDescription.bindings.data();
		pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = vertexDescription.bindings.size();

		_offscrPipelineBuilder = pipelineBuilder;
		_shadowPipelineBuilder = pipelineBuilder;

		
		pipelineBuilder._depthStencil = vkinit::depth_stencil_create_info(false, false, VK_COMPARE_OP_LESS_OR_EQUAL);

		VertexInputDescription outputQuadVertDescription = Plane::get_vertex_description();

		pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();
		pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = outputQuadVertDescription.bindings.size();
		pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = outputQuadVertDescription.bindings.data();
		pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = outputQuadVertDescription.attributes.size();
		pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = outputQuadVertDescription.attributes.data();
		_postprocessingPipelineBuilder = pipelineBuilder;
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
				case 1:
					stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			}

			auto it = _shaderCache.find(shaderPaths[i]);
			if (it != _shaderCache.end())
			{
				shaderEffect->add_stage(it->second, stage);
			}
			else
			{
				Shader* temp = new Shader{_engine->_device};
				temp->load_shader_module(shaderPaths[i].c_str());
				_shaderCache[shaderPaths[i]] = temp;
				shaderEffect->add_stage(it->second, stage);
			}
		}

		shaderEffect->setLayouts.push_back(_engine->_globalSetLayout);
		shaderEffect->setLayouts.push_back(_engine->_objectSetLayout);
		shaderEffect->setLayouts.push_back(_engine->_texturesSetLayout);

		return shaderEffect;
	}

	void setup_shader_stages(PipelineBuilder& builder, std::vector<ShaderEffect::ShaderStage>& stages)
	{
		for (auto& stage : stages)
		{
			builder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(stage.stage, stage.shader->get_shader_module()));
		}
	}
	
	ShaderPass* MaterialSystem::build_shader_pass(VkRenderPass renderPass, PipelineBuilder& builder, ShaderEffect* effect)
	{
		ShaderPass* shaderPass = new ShaderPass();

		shaderPass->effect = effect;
		
	    VkPipelineLayoutCreateInfo info = vkinit::pipeline_layout_create_info();
	    info.setLayoutCount = effect->setLayouts.size();
	    info.pSetLayouts = effect->setLayouts.data();
	    VkPipelineLayout layout;
	    VkResult res = vkCreatePipelineLayout(_engine->_device, &info, nullptr, &layout);
	    if (res != VK_SUCCESS)
			LOG_ERROR("Error creating pipeline layout in build_shader_pass");

		builder._pipelineLayout = layout;
	    setup_shader_stages(builder, effect->stages);

	    VkPipeline pipeline = builder.build_pipeline(_engine->_device, renderPass);

		return shaderPass;
	}

	void MaterialSystem::build_default_templates()
	{
		setup_pipeline_builders();
	
		ShaderEffect* texturedLitEffect = build_shader_effect({
			"shaders/mesh.vert.spv",
			"shaders/textured_lig.frag.spv" });
		ShaderEffect* postprocessingEffect = build_shader_effect({
		    "shaders/postprocessing.vert.spv",
			"shaders/postprocessing.frag.spv"});
		ShaderEffect* coloredLitEffect = build_shader_effect({
			"shaders/mesh.vert.spv"
			"shaders/default_lig.frag.spv"});

		ShaderPass* texturedLitPass = build_shader_pass(_engine->_offscrRenderPass, _offscrPipelineBuilder, texturedLitEffect);
		ShaderPass* postrpocessingPass = build_shader_pass(_engine->_renderPass, _postprocessingPipelineBuilder, postprocessingEffect);
		ShaderPass* coloredLitPass = build_shader_pass(_engine->_offscrRenderPass, _offscrPipelineBuilder, coloredLitEffect);

		EffectTemplate effectTemplate;
		effectTemplate.passShaders[MeshpassType::DirectionalShadow] = nullptr;
		effectTemplate.passShaders[MeshpassType::Transparency] = nullptr;
		effectTemplate.passShaders[MeshpassType::Forward] = texturedLitPass;
		effectTemplate.defaultParameters = nullptr;
		effectTemplate.transparency = assets::MaterialMode::OPAQUE;
		_templateCache["PBR_opaque"] = effectTemplate;

		effectTemplate.passShaders[MeshpassType::Forward] = postrpocessingPass;
		_templateCache["Postprocessing"] = effectTemplate;
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
			newMat->passSets[MeshpassType::DirectionalShadow] = VK_NULL_HANDLE;
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

	void MaterialSystem::clenaup()
	{
		
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
				if (textures[i].view != other.textures[i].view)
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
			size_t texture_hash = (std::hash<size_t>()((size_t)tex.sampler) << 3) && (std::hash<size_t>()((size_t)tex.view) >> 7);

			result ^= std::hash<size_t>()(texture_hash);
		}

		return result;
	}
}
