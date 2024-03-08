#pragma once

#include "engine_core/object.h"
#include "core/math_base.h"

namespace ad_astris::ecore
{
	struct ModelInfo
	{
		std::vector<uint32_t> indices;
		std::vector<XMFLOAT3> vertexPositions;
		std::vector<XMFLOAT3> vertexNormals;
		std::vector<XMFLOAT4> vertexTangents;
		std::vector<XMFLOAT2> vertexUVSet0;
		std::vector<XMFLOAT2> vertexUVSet1;
		std::vector<XMUINT4> vertexBoneIndices;
		std::vector<XMFLOAT4> vertexBoneWeights;
		std::vector<XMFLOAT2> vertexAtlas;
		std::vector<uint32_t> vertexColors;
		std::vector<uint8_t> vertexWindWeights;

		struct Mesh
		{
			uint32_t indexCount{ 0 };
			uint32_t indexOffset{ 0 };
		};

		std::vector<Mesh> meshes;
	};

	struct ModelSphereBounds
	{
		XMFLOAT3 origin;
		float radius;
	};
	
	class Model : public Object
	{
		public:
			ModelInfo& get_model_info() { return _modelInfo; }
			const ModelInfo& get_model_info() const { return _modelInfo; }
 		
		private:
			ModelInfo _modelInfo;
			ModelSphereBounds _sphereBounds;
	};
}
