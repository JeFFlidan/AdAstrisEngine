#pragma once

#include "vk_types.h"
#include "logger.h"
#include "vk_shaders.h"
#include <material_asset.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <array>
#include <vulkan/vulkan_core.h>

class VulkanEngine;
namespace vkutil
{
	class PipelineBuilder
	{
		public:
			std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
			VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
			VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
			VkViewport _viewport;
			VkRect2D _scissor;
			VkPipelineDepthStencilStateCreateInfo _depthStencil;
			VkPipelineRasterizationStateCreateInfo _rasterizer;
			VkPipelineMultisampleStateCreateInfo _multisampling;
			VkPipelineColorBlendAttachmentState _colorBlendAttachment;
			VkPipelineLayout _pipelineLayout;

			VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
	};

	
	enum class MeshpassType : uint32_t
	{
		None = 0,
		Forward = 1,
		Transparency = 2,
		DirectionalShadow = 3,
	};
	
	struct ShaderParameters
	{
		std::string demo;
	};

	struct SampledTexture
	{
		VkSampler sampler;
		VkImageView view;
	};

	template<typename T>
	struct PerPassData
	{
		public:
			T& operator[](MeshpassType pass)
			{
				switch (pass)
				{
					case MeshpassType::None:
						LOG_WARNING("No information about material meshpass");
						break;
					case MeshpassType::Forward:
						return data[0];
					case MeshpassType::Transparency:
						return data[1];
					case MeshpassType::DirectionalShadow:
						return data[2];
				}

				return data[0];
			}

			void clear(T&& val)
			{
				for (int i = 0; i != 3; ++i)
					data[i] = val;
			}

		private:
			std::array<T, 3> data;
	};

	struct ShaderEffect
	{
		// Storage information to create a VkPipeline
		//VkPipelineLayout buildLayout;
		std::vector<VkDescriptorSetLayout> setLayouts;

		struct ShaderStage
		{
			Shader* shader;
			VkShaderStageFlagBits stage;
		};

		std::vector<ShaderStage> stages;

		void add_stage(Shader* shader, VkShaderStageFlagBits shaderStage)
		{
			stages.push_back({shader, shaderStage});
		}
	};

	struct ShaderPass
	{
		ShaderEffect* effect{ nullptr };
		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout layout{ VK_NULL_HANDLE };
	};

	struct EffectTemplate
	{
		PerPassData<ShaderPass*> passShaders;

		ShaderParameters* defaultParameters;
		assets::MaterialMode transparency;
	};

	struct Material
	{
		EffectTemplate* original;
		PerPassData<VkDescriptorSet> passSets;

		std::vector<SampledTexture> textures;
		ShaderParameters parameters;
	};
	
	struct MaterialData
	{
		std::vector<SampledTexture> textures;
		std::string baseTemplate;
		ShaderParameters parameters;

		bool operator==(const MaterialData& othero) const;
	
		size_t hash() const;
	};

	class MaterialSystem
	{
		public:
			void init(VulkanEngine* engine);
			void clenaup();

			void build_default_templates();

			ShaderPass* build_shader_pass(VkRenderPass renderPass, PipelineBuilder& builder, ShaderEffect* effect);
			ShaderEffect* build_shader_effect(const std::vector<std::string>& shaderPaths);
			
			Material* build_material(const std::string& materialName, const MaterialData& info);
			Material* get_material(const std::string& materialName);

			void setup_pipeline_builders();
		
		private:
			struct MaterialInfoHash
			{
				std::size_t operator()(const MaterialData& k) const
				{
					return k.hash();
				}
			};

			PipelineBuilder _postprocessingPipelineBuilder;
			PipelineBuilder _offscrPipelineBuilder;
			PipelineBuilder _shadowPipelineBuilder;

			std::unordered_map<std::string, EffectTemplate> _templateCache;
			std::unordered_map<std::string, Material*> _materials;
			std::unordered_map<std::string, Shader*> _shaderCache;
			std::unordered_map<MaterialData, Material*, MaterialInfoHash> _materialCache;
			VulkanEngine* _engine;
	};
}
