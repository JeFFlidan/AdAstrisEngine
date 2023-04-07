#pragma once

#include "rhi/engine_rhi.h"
#include "rhi/resources.h"
#include "glm/vec4.hpp"
#include <vector>
#include <map>
#include <array>

namespace ad_astris::rcore
{
	enum class MeshPassType
	{
		DEFERRED_MRT,
		DEFERRED_LIGHTING,
		OIT_GEOMETRY,
		OIT_SOLVE,
		TAA,
		POSTPROCESSING,
		POINT_SHADOW,
		SPOT_SHADOW,
		DIRECTIONAL_SHADOW
	};
	
	template<typename T>
	class MeshPassesData
	{
		public:
			T& operator[](MeshPassType type)
			{
				switch (type)
				{
					case MeshPassType::DEFERRED_MRT:
						return data[0];
					case MeshPassType::DEFERRED_LIGHTING:
						return data[1];
					case MeshPassType::OIT_GEOMETRY:
						return data[2];
					case MeshPassType::OIT_SOLVE:
						return data[3];
					case MeshPassType::TAA:
						return data[4];
					case MeshPassType::POSTPROCESSING:
						return data[5];
					case MeshPassType::POINT_SHADOW:
						return data[6];
					case MeshPassType::SPOT_SHADOW:
						return data[7];
					case MeshPassType::DIRECTIONAL_SHADOW:
						return data[8];
				}
				return data[0];
			}

			void fill_passes_data(T&& val)
			{
				for (int i = 0; i != data.size(); ++i)
				{
					data[i] = val;
				}
			}

		private:
			std::array<T, 9> data;
	};
	
	struct ShaderPass
	{
		std::vector<rhi::Shader*> shaderStages;
		rhi::Pipeline pipeline;
		rhi::RenderPass renderPass;
	};
	
	struct MaterialTemplate
	{
		MeshPassesData<ShaderPass*> meshPassShaders;
	};

	struct DeferredParameters
	{
		glm::vec4 unused1;
	};

	struct TransparentParameters
	{
		glm::vec4 unused1;
	};

	struct PostprocessingParameters
	{
		glm::vec4 unused1;
	};

	template<typename T>
	struct Material
	{
		
		MaterialTemplate materialTemplate;
		T materialParameters;
	};

	class MaterialSystem
	{
		public:
			MaterialSystem(rhi::IEngineRHI* rhi);

			
		private:
			rhi::IEngineRHI* _rhi;
			std::map<MeshPassType, MaterialTemplate> _materialTemplates;
			std::vector<Material<DeferredParameters>> _deferredLightingMaterials;
			std::vector<Material<PostprocessingParameters>> _postprocessingMaterials;
			std::vector<Material<TransparentParameters>> _transparentMaterials;
	};
}