#include "model.h"
#include "profiler/logger.h"
#include "core/constants.h"
#include "core/custom_objects_to_json.h"
#include <json/json.hpp>

using namespace ad_astris;
using namespace ecore;

constexpr const char* UUID_KEY = "uuid";
constexpr const char* ORIGINAL_FILE_KEY = "original_file";
constexpr const char* MATERIAL_NAMES_KEY = "material_names";
constexpr const char* MATERIAL_INDEX_KEY = "material_index";
constexpr const char* MESHES_KEY = "meshes";
constexpr const char* MODEL_NAME_KEY = "name";
constexpr const char* SPHERE_BOUNDS_RADIUS_KEY = "sphere_bounds_radius";
constexpr const char* SPHERE_BOUNDS_ORIGIN_KEY = "sphere_bounds_origin";
constexpr const char* INDEX_COUNT_KEY = "index_count";
constexpr const char* INDEX_OFFSET_KEY = "index_offset";
constexpr const char* VERTEX_COUNT_KEY = "vertex_count";
constexpr const char* UV_SET0_OFFSET_KEY = "uv_set0_offset";
constexpr const char* UV_SET1_OFFSET_KEY = "uv_set1_offset";
constexpr const char* BONE_INDICES_OFFSET_KEY = "bone_indices_offset";
constexpr const char* BONE_WEIGHTS_OFFSET_KEY = "bone_weights_offset";
constexpr const char* ATLAS_OFFSET_KEY = "atlas_offset";
constexpr const char* COLORS_OFFSET_KEY = "colors_offset";
constexpr const char* WIND_WEIGHTS_KEY = "wind_weights_offset";

template<typename T>
size_t get_byte_size(const std::vector<T>& vec)
{
	return vec.size() * sizeof(T);
}

template<typename T>
void add_data_to_blob(uint8_t* dstBlob, const std::vector<T>& src, uint64_t& offset)
{
	memcpy(dstBlob + offset, src.data(), get_byte_size(src));
	offset += get_byte_size(src);
}

template<typename T>
uint64_t get_data_from_blob(uint8_t* srcBlob, std::vector<T>& dst, uint64_t vertexCount, uint64_t offset)
{
	dst.resize(vertexCount);
	memcpy(dst.data(), srcBlob + offset, vertexCount * sizeof(T));
	return offset + vertexCount * sizeof(T);
}

Model::Model(const ModelInfo& modelInfo) : _modelInfo(modelInfo)
{
	
}

void Model::serialize(io::File* file)
{
	if (_modelInfo.indices.empty() && _modelInfo.vertexPositions.empty() && _modelInfo.vertexNormals.empty() && _modelInfo.vertexTangents.empty())
	{
		LOG_ERROR("Model::serialize(): Failed to serialize model because it does not have indices, positions, normals and tangents.")
		return;
	}
	
	nlohmann::json metadata;
	metadata[UUID_KEY] = _modelInfo.uuid;
	metadata[ORIGINAL_FILE_KEY] = _modelInfo.originalFile;
	metadata[VERTEX_COUNT_KEY] = _modelInfo.vertexPositions.size();
	metadata[INDEX_COUNT_KEY] = _modelInfo.indices.size();
	metadata[MATERIAL_NAMES_KEY] = _modelInfo.materialNames;
	metadata[SPHERE_BOUNDS_ORIGIN_KEY] = _modelInfo.sphereBounds.origin;
	metadata[SPHERE_BOUNDS_RADIUS_KEY] = _modelInfo.sphereBounds.radius;

	nlohmann::json meshesJson;
	size_t meshCounter = 0;
	for (auto& mesh : _modelInfo.meshes)
	{
		nlohmann::json meshJson;
		meshJson[INDEX_COUNT_KEY] = mesh.indexCount;
		meshJson[INDEX_OFFSET_KEY] = mesh.indexOffset;
		meshJson[MATERIAL_INDEX_KEY] = mesh.materialIndex;
		meshesJson["mesh" + std::to_string(meshCounter++)] = meshJson;
	}
	metadata[MESHES_KEY] = meshesJson;

	if (!_modelInfo.name.empty())
		metadata[MODEL_NAME_KEY] = _modelInfo.name;
	
	uint64_t offset = 0;
	const uint64_t blobSize = get_size();
	uint8_t* blob = new uint8_t[blobSize];
	add_data_to_blob(blob, _modelInfo.indices, offset);
	add_data_to_blob(blob, _modelInfo.vertexPositions, offset);
	add_data_to_blob(blob, _modelInfo.vertexNormals, offset);
	add_data_to_blob(blob, _modelInfo.vertexTangents, offset);
	
	if (!_modelInfo.vertexUVSet0.empty())
	{
		metadata[UV_SET0_OFFSET_KEY] = offset;
		add_data_to_blob(blob, _modelInfo.vertexUVSet0, offset);
	}
	if (!_modelInfo.vertexUVSet1.empty())
	{
		metadata[UV_SET1_OFFSET_KEY] = offset;
		add_data_to_blob(blob, _modelInfo.vertexUVSet1, offset);
	}
	if (!_modelInfo.vertexBoneIndices.empty())
	{
		metadata[BONE_INDICES_OFFSET_KEY] = offset;
		add_data_to_blob(blob, _modelInfo.vertexBoneIndices, offset);
	}
	if (!_modelInfo.vertexBoneWeights.empty())
	{
		metadata[BONE_WEIGHTS_OFFSET_KEY] = offset;
		add_data_to_blob(blob, _modelInfo.vertexBoneWeights, offset);
	}
	if (!_modelInfo.vertexAtlas.empty())
	{
		metadata[ATLAS_OFFSET_KEY] = offset;
		add_data_to_blob(blob, _modelInfo.vertexAtlas, offset);
	}
	if (!_modelInfo.vertexColors.empty())
	{
		metadata[COLORS_OFFSET_KEY] = offset;
		add_data_to_blob(blob, _modelInfo.vertexColors, offset);
	}
	if (!_modelInfo.vertexWindWeights.empty())
	{
		metadata[WIND_WEIGHTS_KEY] = offset;
		add_data_to_blob(blob, _modelInfo.vertexWindWeights, offset);
	}
	
	file->set_binary_blob(blob, blobSize);
	file->set_metadata(metadata.dump(JSON_INDENT));
}

void Model::deserialize(io::File* file, ObjectName* objectName)
{
	_path = file->get_file_path();
	_name = objectName;

	nlohmann::json metadata = nlohmann::json::parse(file->get_metadata());
	_modelInfo.uuid = metadata[UUID_KEY];
	_modelInfo.originalFile = metadata[ORIGINAL_FILE_KEY];
	_modelInfo.materialNames = metadata[MATERIAL_NAMES_KEY].get<std::vector<std::string>>();
	_modelInfo.sphereBounds.origin = metadata[SPHERE_BOUNDS_ORIGIN_KEY];
	_modelInfo.sphereBounds.radius = metadata[SPHERE_BOUNDS_RADIUS_KEY];
	if (metadata.contains(MODEL_NAME_KEY))
		_modelInfo.name = metadata[MODEL_NAME_KEY];

	nlohmann::json meshesJson = metadata[MESHES_KEY];
	for (auto& pair : meshesJson.items())
	{
		ModelInfo::Mesh& mesh = _modelInfo.meshes.emplace_back();
		nlohmann::json meshJson = pair.value();
		mesh.indexCount = meshJson[INDEX_COUNT_KEY];
		mesh.indexOffset = meshJson[INDEX_OFFSET_KEY];
		mesh.materialIndex = meshJson[MATERIAL_INDEX_KEY];
	}

	uint64_t offset = 0;
	uint8_t* srcBlob = file->get_binary_blob();
	_modelInfo.indices.resize(metadata[INDEX_COUNT_KEY]);
	memcpy(_modelInfo.indices.data(), srcBlob, get_byte_size(_modelInfo.indices));
	offset += get_byte_size(_modelInfo.indices);
	const uint64_t vertexCount = metadata[VERTEX_COUNT_KEY];
	offset = get_data_from_blob(srcBlob, _modelInfo.vertexPositions, vertexCount, offset);
	offset = get_data_from_blob(srcBlob, _modelInfo.vertexNormals, vertexCount, offset);
	get_data_from_blob(srcBlob, _modelInfo.vertexTangents, vertexCount, offset);

	if (metadata.contains(UV_SET0_OFFSET_KEY))
		get_data_from_blob(srcBlob, _modelInfo.vertexUVSet0, vertexCount, metadata[UV_SET0_OFFSET_KEY]);
	if (metadata.contains(UV_SET1_OFFSET_KEY))
		get_data_from_blob(srcBlob, _modelInfo.vertexUVSet1, vertexCount, metadata[UV_SET1_OFFSET_KEY]);
	if (metadata.contains(BONE_INDICES_OFFSET_KEY))
		get_data_from_blob(srcBlob, _modelInfo.vertexBoneIndices, vertexCount, metadata[BONE_INDICES_OFFSET_KEY]);
	if (metadata.contains(BONE_WEIGHTS_OFFSET_KEY))
		get_data_from_blob(srcBlob, _modelInfo.vertexBoneWeights, vertexCount, metadata[BONE_WEIGHTS_OFFSET_KEY]);
	if (metadata.contains(ATLAS_OFFSET_KEY))
		get_data_from_blob(srcBlob, _modelInfo.vertexAtlas, vertexCount, metadata[ATLAS_OFFSET_KEY]);
	if (metadata.contains(COLORS_OFFSET_KEY))
		get_data_from_blob(srcBlob, _modelInfo.vertexColors, vertexCount, metadata[COLORS_OFFSET_KEY]);
	if (metadata.contains(WIND_WEIGHTS_KEY))
		get_data_from_blob(srcBlob, _modelInfo.vertexWindWeights, vertexCount, metadata[WIND_WEIGHTS_KEY]);
}

uint64_t Model::get_size()
{
	uint64_t size = get_byte_size(_modelInfo.indices);
	size += get_byte_size(_modelInfo.vertexPositions);
	size += get_byte_size(_modelInfo.vertexNormals);
	size += get_byte_size(_modelInfo.vertexTangents);
	size += get_byte_size(_modelInfo.vertexUVSet0);
	size += get_byte_size(_modelInfo.vertexUVSet1);
	size += get_byte_size(_modelInfo.vertexBoneIndices);
	size += get_byte_size(_modelInfo.vertexBoneWeights);
	size += get_byte_size(_modelInfo.vertexAtlas);
	size += get_byte_size(_modelInfo.vertexColors);
	size += get_byte_size(_modelInfo.vertexWindWeights);
	return size;
}

void Model::accept(resource::IResourceVisitor& resourceVisitor)
{
	// TODO
}
