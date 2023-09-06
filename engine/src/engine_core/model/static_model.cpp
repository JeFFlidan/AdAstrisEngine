#include "static_model.h"
#include "resource_manager/resource_visitor.h"
#include "profiler/logger.h"

#include <json.hpp>

using namespace ad_astris;

ecore::StaticModelData ecore::StaticModel::get_model_data()
{
	StaticModelData data;
	data.indexBuffer = _indexBuffer;
	data.vertexBuffer = _vertexBuffer;
	data.vertexBufferSize = _modelInfo.vertexBufferSize;
	data.indexBufferSize = _modelInfo.indexBufferSize;
	return data;
}

inline ecore::model::ModelBounds ecore::StaticModel::get_model_bounds()
{
	return _modelInfo.bounds;
}

ecore::model::VertexFormat ecore::StaticModel::get_vertex_format()
{
	return _modelInfo.vertexFormat;
}

inline std::string ecore::StaticModel::get_original_file()
{
	return _modelInfo.originalFile;
}

inline std::vector<std::string> ecore::StaticModel::get_materials_name()
{
	return _modelInfo.materialsName;
}

void ecore::StaticModel::serialize(io::IFile* file)
{
	std::string newMetadata = model::Utils::pack_static_model_info(&_modelInfo);
	file->set_metadata(newMetadata);
}

void ecore::StaticModel::deserialize(io::IFile* file, ObjectName* objectName)
{
	std::string strMetadata = file->get_metadata();
	_modelInfo = model::Utils::unpack_static_model_info(strMetadata);
	
	uint8_t* buffer = file->get_binary_blob();
	_vertexBuffer = buffer;
	_indexBuffer = buffer + _modelInfo.vertexBufferSize;
	_name = objectName;
	_path = file->get_file_path();
}

inline uint64_t ecore::StaticModel::get_size()
{
	return _modelInfo.vertexBufferSize + _modelInfo.indexBufferSize;
}

inline bool ecore::StaticModel::is_resource()
{
	return true;
}

inline UUID ecore::StaticModel::get_uuid()
{
	return _modelInfo.uuid;
}

inline std::string ecore::StaticModel::get_description()
{
	// TODO
	return std::string();
}

inline std::string ecore::StaticModel::get_type()
{
	return "model";
}

inline void ecore::StaticModel::accept(resource::IResourceVisitor& resourceVisitor)
{
	resourceVisitor.visit(this);
}

void io::ConversionContext<ecore::StaticModel>::get_data(std::string& metadata, uint8_t*& binBlob, uint64_t& binBlobSize, URI& path)
{
	ecore::model::StaticModelInfo info;
	info.uuid = uuid;
	info.vertexBufferSize = vertexBufferSize;
	info.indexBufferSize = indexBufferSize;
	info.originalFile = originalFile;
	info.bounds = modelBounds;
	info.materialsName = materialsName;
	info.vertexFormat = vertexFormat;

	metadata = ecore::model::Utils::pack_static_model_info(&info);

	binBlobSize = vertexBufferSize + indexBufferSize;
	path = filePath.c_str();
	binBlob = buffer;
}
