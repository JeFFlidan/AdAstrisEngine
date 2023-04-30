#pragma once

#include "vulkan_renderer/vk_pipeline.h"
#include "vulkan_renderer/vk_mesh.h"
#include "vulkan_renderer/vk_types.h"
#include "profiler/logger.h"
#include "shader.h"
#include <material_asset.h>

#include <vector>
#include <string>
#include <unordered_map>
#include <array>
#include <vulkan/vulkan_core.h>

class VkRenderer;
namespace ad_astris
{
	enum class MeshpassType : uint32_t
	{
		None = 0,
		Forward = 1,
		Transparency = 2,
		DirectionalShadow = 3,
		PointShadow = 4,
		SpotShadow = 5,
		Deferred = 6,
	};
	
	struct ShaderParameters
	{
		std::string demo;
	};

	struct SampledTexture
	{
		VkSampler sampler;
		VkImageView imageView;
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
					case MeshpassType::PointShadow:
						return data[3];
					case MeshpassType::SpotShadow:
						return data[4];
					case MeshpassType::Deferred:
						return data[5];
				}

				return data[0];
			}

			void clear(T&& val)
			{
				for (int i = 0; i != 6; ++i)
					data[i] = val;
			}

		private:
			std::array<T, 6> data;
	};

	struct ShaderPass
	{
		ShaderEffect* effect{ nullptr };
		VkPipeline pipeline{ VK_NULL_HANDLE };
		VkPipelineLayout layout{ VK_NULL_HANDLE };
		VkRenderPass renderPass{ VK_NULL_HANDLE };
		GraphicsPipelineBuilder* pipelineBuilder{ nullptr };
		std::vector<ShaderPass*> relatedShaderPasses;	// Temporary solution to test Deferred rendering. Need to remake in the future
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
		//PerPassData<VkDescriptorSet> passSets;

		std::vector<SampledTexture> textures;
		ShaderParameters parameters;
	};
	
	struct MaterialData
	{
		std::vector<SampledTexture> textures;
		std::string baseTemplate;
		ShaderParameters parameters;

		bool operator==(const MaterialData& other) const;
	
		size_t hash() const;
	};

	class MaterialSystem
	{
		public:
			void init(VkRenderer* engine);
			void cleanup();

			void build_default_templates();

			ShaderPass* build_shader_pass(VkRenderPass& renderPass, GraphicsPipelineBuilder& builder, ShaderEffect* effect);
			ShaderEffect* build_shader_effect(const std::vector<std::string>& shaderPaths);
			
			Material* build_material(const std::string& materialName, const MaterialData& info);
			Material* get_material(const std::string& materialName);
		
		private:
			struct MaterialInfoHash
			{
				std::size_t operator()(const MaterialData& k) const
				{
					return k.hash();
				}
			};

			GraphicsPipelineBuilder _postprocessingPipelineBuilder;
			GraphicsPipelineBuilder _offscrPipelineBuilder;
			GraphicsPipelineBuilder _GBufferPipelineBuilder;
			GraphicsPipelineBuilder _deferredPipelineBuilder;
			GraphicsPipelineBuilder _dirShadowPipelineBuilder;
			GraphicsPipelineBuilder _pointShadowPipelineBuilder;
			GraphicsPipelineBuilder _spotShadowPipelineBuilder;
			GraphicsPipelineBuilder _transparencyBuilder;
			GraphicsPipelineBuilder _compositePipelineBuilder;

			std::unordered_map<std::string, EffectTemplate> _templateCache;
			std::unordered_map<std::string, Material*> _materials;
			std::unordered_map<std::string, Shader*> _shaderCache;
			std::unordered_map<MaterialData, Material*, MaterialInfoHash> _materialCache;
			VkRenderer* _engine;
	
			void setup_pipeline_builders();
	};
}
