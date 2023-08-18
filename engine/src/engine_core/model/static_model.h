#pragma once

#include "model_common.h"
#include "engine_core/object.h"

namespace ad_astris::ecore
{
	struct StaticModelData
	{
		uint64_t vertexBufferSize;
		uint8_t* vertexBuffer;
		uint64_t indexBufferSize;
		uint8_t* indexBuffer;
	};
	
	class StaticModel : public Object
	{
		friend model::Utils;
		
		public:
			StaticModel() = default;

			StaticModelData get_model_data();
			model::ModelBounds get_model_bounds();
			model::VertexFormat get_vertex_format();
			std::string get_original_file();
			std::vector<std::string> get_materials_name();
		
		public:
			// ========== Begin Object interface ==========
			
			virtual void serialize(io::IFile* file) override;
			virtual void deserialize(io::IFile* file, ObjectName* objectName) override;
			virtual uint64_t get_size() override;
			virtual bool is_resource() override;
			virtual UUID get_uuid() override;
			virtual std::string get_description() override;
			virtual std::string get_type() override;
			virtual void accept(resource::IResourceVisitor& resourceVisitor) override;

			// ========== End Object interface ==========

		public:
			

		private:
			model::StaticModelInfo _modelInfo;
			uint8_t* _vertexBuffer{ nullptr };
			uint8_t* _indexBuffer{ nullptr };
	};

}

namespace ad_astris::io
{
	template<>
	struct ConversionContext<ecore::StaticModel>
	{
		UUID uuid;
		uint8_t* buffer{ nullptr };
		uint64_t vertexBufferSize;
		uint64_t indexBufferSize;
		std::string originalFile;
		std::string filePath;
		ecore::model::ModelBounds modelBounds;
		ecore::model::VertexFormat vertexFormat;
		std::vector<std::string> materialsName;
		
		void get_data(std::string& metadata,uint8_t*& binBlob,uint64_t& binBlobSize, URI& path);
	};
}