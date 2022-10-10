#pragma once

#include <vk_types.h>
#include <material_asset.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <array>

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
		VkPipelineLayout buildLayout;
		std::array<VkDescriptorSetLayout, 4> setLayouts;

		struct ShaderStage
		{
			VkShaderModule* shaderModule;
			VkShaderStageFlagBits stage;
		};

		std::vector<ShaderStage> stages;
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
	};

	struct Material
	{
		EffectTemplate* original;
		PerPassData<VkDescriptorSet> passSets;

		std::vector<SampledTexture> textures;
	};
	
	struct MaterialData
	{
		std::vector<SampledTexture> textures;
		std::string baseTemplate;

		bool operator==(const MaterialData& other) const;
	
		size_t hash() const;
	};

	class MaterialSystem
	{
		public:

			void init(VulkanEngine* engine);
			void clenaup();

			void build_default_templates();

			ShaderPass* build_shader(VkRenderPass renderPass, PipelineBuilder& builder, ShaderEffect* effect);

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

			PipelineBuilder forwardPipelineBuilder;
			PipelineBuilder shadowPipelineBuilder;

			std::unordered_map<std::string, EffectTemplate> templateCache;
			std::unordered_map<std::string, Material*> materials;
			std::unordered_map<MaterialData, Material*, MaterialInfoHash> materialCache;
			VulkanEngine* engine;
	};
}
