#pragma once

#include "engine_core/object.h"
#include "core/math_base.h"

namespace ad_astris::ecore
{
	struct SphereBounds
	{
		XMFLOAT3 origin{ 0.0f, 0.0f, 0.0f };
		float radius{ 1.0f };
	};
	
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

		std::vector<std::string> materialNames;

		struct Mesh
		{
			uint32_t indexCount{ 0 };
			uint32_t indexOffset{ 0 };
			uint32_t materialIndex{ 0 };
		};

		std::vector<Mesh> meshes;

		SphereBounds sphereBounds;
		
		std::string originalFile;
		std::string name;	// This field is used when loaded model is not merged by engine 
	};
	
	class Model : public Object
	{
		public:
			Model() = default;
			Model(const ModelInfo& modelInfo);
		
			ModelInfo& get_model_info() { return _modelInfo; }
			const ModelInfo& get_model_info() const { return _modelInfo; }
			SphereBounds& get_sphere_bounds() { return _modelInfo.sphereBounds; }
			const SphereBounds& get_sphere_bounds() const { return _modelInfo.sphereBounds; }
		
			// ========== Begin Object interface ==========
				
			void serialize(io::File* file) override;
			void deserialize(io::File* file, ObjectName* objectName) override;
			uint64_t get_size() override;
			void accept(resource::IResourceVisitor& resourceVisitor) override;
			bool is_resource() override { return true; }
			UUID get_uuid() override { return _uuid; }
			std::string get_description() override { return "Model"; }
			std::string get_type() override { return "model"; }

			// ========== End Object interface ==========
 		
		private:
			ModelInfo _modelInfo;
			UUID _uuid;
	};

	struct ModelConversionContext
	{
		bool mergeMeshes{ true };
		bool loadTextures{ true };
		bool generateMaterials{ false };
		bool generateTangents{ false };
	};
}
