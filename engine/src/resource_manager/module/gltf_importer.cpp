#include "gltf_importer.h"
#include "file_system/utils.h"
#include "core/global_objects.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_FS
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include <tinygltf/tiny_gltf.h>

using namespace ad_astris;
using namespace resource::impl;

namespace tinygltf
{
	bool FileExists(const std::string& absFilename, void*)
	{
		return io::Utils::exists(absFilename);
	}

	std::string ExpandFilePath(const std::string& filePath, void*)
	{
#ifdef _WIN32
		DWORD len = ExpandEnvironmentStringsA(filePath.c_str(), NULL, 0);
		std::string result;
		result.resize(len);
		ExpandEnvironmentStringsA(filePath.c_str(), result.data(), len);
		return result;
#endif
	}

	bool ReadWholeFile(std::vector<unsigned char>* out, std::string* err, const std::string& filePath, void*)
	{
		io::Utils::read_file(FILE_SYSTEM(), filePath, *out);
		return true;
	}

	bool WriteWholeFile(
		std::string* err,
		const std::string& filePath,
		const std::vector<unsigned char>& contents, void*)
	{
		// TODO
		assert(0 && "WriteWholeFile is not implemented");
		return true;
	}

	bool LoadImageData(Image *image, const int imageIdx, std::string *err, std::string *warn,
		int reqWidth, int reqHeight, const unsigned char *bytes, int size, void *userData)
	{
		// TODO
		assert(0 && "LoadImageData is not implemented");
		return true;
	}

	bool WriteImageData(const std::string *basePath, const std::string *fileName, const Image *image,
		bool embedImages, const URICallbacks *uriCb, std::string *outUri, void *userPointer)
	{
		// TODO
		assert(0 && "WriteImageData is not implemented");
		return false;
	}
}

bool GLTFImporter::import(
	const std::string& path,
	std::vector<ModelCreateInfo>& outModelInfos,
	std::vector<TextureCreateInfo>& outTextureInfos,
	std::vector<MaterialCreateInfo>& materialInfos,
	const ecore::ModelConversionContext& conversionContext)
{
	tinygltf::TinyGLTF loader;
	std::string tinygltfError;
	std::string tinygltfWarning;
	
	tinygltf::FsCallbacks callbacks{};
	callbacks.ReadWholeFile = tinygltf::ReadWholeFile;
	callbacks.WriteWholeFile = tinygltf::WriteWholeFile;
	callbacks.ExpandFilePath = tinygltf::ExpandFilePath;
	callbacks.FileExists = tinygltf::FileExists;
	
	loader.SetFsCallbacks(callbacks);

	loader.SetImageLoader(tinygltf::LoadImageData, nullptr);
	loader.SetImageWriter(tinygltf::WriteImageData, nullptr);
	
	std::vector<uint8_t> fileData;
	io::Utils::read_file(FILE_SYSTEM(), path, fileData);

	std::string extension = io::Utils::get_file_extension(path);
	std::string baseDir = tinygltf::GetBaseDir(path);

	bool isValid = false;
	tinygltf::Model gltfModel;
	
	if (extension == "gltf")
	{
		isValid = loader.LoadASCIIFromString(
			&gltfModel,
			&tinygltfError,
			&tinygltfWarning,
			reinterpret_cast<const char*>(fileData.data()),
			static_cast<uint32_t>(fileData.size()),
			baseDir);
	}
	else
	{
		isValid = loader.LoadBinaryFromMemory(
			&gltfModel,
			&tinygltfError,
			&tinygltfWarning,
			fileData.data(),
			static_cast<uint32_t>(fileData.size()),
			baseDir);
	}

	if (!tinygltfWarning.empty())
	{
		LOG_WARNING("GLTFImporter::import(): {}", tinygltfWarning)
	}

	if (!isValid)
	{
		LOG_ERROR("GLTFImporter::import(): Failed to import GLTF model. {}", tinygltfError)
	}

	for (auto& material : gltfModel.materials)
	{
		
	}

	// From WickedEngine
	const size_t indexRemap[] = { 0, 2, 1 };

	ModelCreateInfo mergedModelCreateInfo;

	for (auto& gltfMesh : gltfModel.meshes)
	{
		ModelCreateInfo& outModelCreateInfo = conversionContext.mergeMeshes ? mergedModelCreateInfo : outModelInfos.emplace_back();
		ecore::ModelInfo& outModelInfo = outModelCreateInfo.info;
		if (!conversionContext.mergeMeshes)
			outModelCreateInfo.name = gltfMesh.name;
		
		for (auto& primitive : gltfMesh.primitives)
		{
			ecore::ModelInfo::Mesh& mesh = outModelInfo.meshes.emplace_back();
			uint32_t vertexOffset = outModelInfo.vertexPositions.size();

			if (primitive.indices >= 0)
			{
				const tinygltf::Accessor& accessor = gltfModel.accessors[primitive.indices];
				const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

				int stride = accessor.ByteStride(bufferView);
				size_t indexCount = accessor.count;
				size_t indexOffset = outModelInfo.indices.size();
				outModelInfo.indices.resize(indexCount + indexOffset);
				mesh.indexCount = indexCount;
				mesh.indexOffset = indexOffset;

				const uint8_t* gltfIndices = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

				if (stride == sizeof(uint8_t))
				{
					for (size_t i = 0; i < indexCount; i += 3)
					{
						outModelInfo.indices[indexOffset + i + 0] = vertexOffset + gltfIndices[i + indexRemap[0]];
						outModelInfo.indices[indexOffset + i + 1] = vertexOffset + gltfIndices[i + indexRemap[1]];
						outModelInfo.indices[indexOffset + i + 2] = vertexOffset + gltfIndices[i + indexRemap[2]];
					}
				}
				else if (stride == sizeof(uint16_t))
				{
					for (size_t i = 0; i < indexCount; i += 3)
					{
						outModelInfo.indices[indexOffset + i + 0] = vertexOffset + ((uint16_t*)gltfIndices)[i + indexRemap[0]];
						outModelInfo.indices[indexOffset + i + 1] = vertexOffset + ((uint16_t*)gltfIndices)[i + indexRemap[1]];
						outModelInfo.indices[indexOffset + i + 2] = vertexOffset + ((uint16_t*)gltfIndices)[i + indexRemap[2]];
					}
				}
				else if (stride == sizeof(uint32_t))
				{
					for (size_t i = 0; i < indexCount; i += 3)
					{
						outModelInfo.indices[indexOffset + i + 0] = vertexOffset + ((uint32_t*)gltfIndices)[i + indexRemap[0]];
						outModelInfo.indices[indexOffset + i + 1] = vertexOffset + ((uint32_t*)gltfIndices)[i + indexRemap[1]];
						outModelInfo.indices[indexOffset + i + 2] = vertexOffset + ((uint32_t*)gltfIndices)[i + indexRemap[2]];
					}
				}
				else
				{
					LOG_ERROR("GLTFImporter::import(): Unsupported stride {}", stride)
					return false;
				}
			}

			for (auto& attribute : primitive.attributes)
			{
				const std::string& attributeName = attribute.first;
				int attributeIndex = attribute.second;

				const tinygltf::Accessor& accessor = gltfModel.accessors[attributeIndex];
				const tinygltf::BufferView& bufferView = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = gltfModel.buffers[bufferView.buffer];

				int stride = accessor.ByteStride(bufferView);
				size_t vertexCount = accessor.count;
				
				if (mesh.indexCount == 0)
				{
					size_t indexOffset = outModelInfo.indices.size();
					outModelInfo.indices.resize(indexOffset + vertexCount);
					for (size_t i = 0; i < vertexCount; i += 3)
					{
						outModelInfo.indices[indexOffset + i + 0] = uint32_t(vertexOffset + i + indexRemap[0]);
						outModelInfo.indices[indexOffset + i + 1] = uint32_t(vertexOffset + i + indexRemap[1]);
						outModelInfo.indices[indexOffset + i + 2] = uint32_t(vertexOffset + i + indexRemap[2]);
					}
					mesh.indexCount = vertexCount;
					mesh.indexOffset = indexOffset;
				}

				const uint8_t* attributeData = buffer.data.data() + accessor.byteOffset + bufferView.byteOffset;

				auto parseUvSet = [attributeData, vertexCount, vertexOffset, stride, &accessor](std::vector<XMFLOAT2>& uvSet)
				{
					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							uvSet[vertexOffset + i] = *(const XMFLOAT2*)(attributeData + i * stride);
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const uint8_t& s = attributeData[i * stride];
							const uint8_t& t = attributeData[i * stride + sizeof(uint8_t)];
							uvSet[vertexOffset + i].x = s / 255.0f;
							uvSet[vertexOffset + i].y = t / 255.0f;
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const uint16_t& s = *(const uint16_t*)(attributeData + i * stride);
							const uint16_t& t = *(const uint16_t*)(attributeData + i * stride + sizeof(uint16_t));
							uvSet[vertexOffset + i].x = s / 65535.0f;
							uvSet[vertexOffset + i].y = t / 65535.0f;
						}
					}
					else
					{
						LOG_ERROR("GLTFImporter::import(): UV set type is unsupported")
						return false;
					}
				};

				if (attributeName == "POSITION")
				{
					outModelInfo.vertexPositions.resize(vertexOffset + vertexCount);
					for (size_t i = 0; i != vertexCount; ++i)
					{
						outModelInfo.vertexPositions[vertexOffset + i] = *(const XMFLOAT3*)(attributeData + i * stride);
					}
				}
				else if (attributeName == "NORMAL")
				{
					outModelInfo.vertexNormals.resize(vertexOffset + vertexCount);
					for (size_t i = 0; i != vertexCount; ++i)
					{
						outModelInfo.vertexNormals[vertexOffset + i] = *(const XMFLOAT3*)(attributeData + i * stride);
					}
				}
				else if (attributeName == "TANGENT")
				{
					outModelInfo.vertexTangents.resize(vertexOffset + vertexCount);
					for (size_t i = 0; i != vertexCount; ++i)
					{
						outModelInfo.vertexTangents[vertexOffset + i] = *(const XMFLOAT4*)(attributeData + i + stride);
					}
				}
				else if (attributeName == "TEXCOORD_0")
				{
					outModelInfo.vertexUVSet0.resize(vertexOffset + vertexCount);
					parseUvSet(outModelInfo.vertexUVSet0);
				}
				else if (attributeName == "TEXCOORD_1")
				{
					outModelInfo.vertexUVSet1.resize(vertexOffset + vertexCount);
					parseUvSet(outModelInfo.vertexUVSet1);
				}
				else if (attributeName == "JOINTS_0")
				{
					outModelInfo.vertexBoneIndices.resize(vertexCount + vertexOffset);

					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
					{
						struct TempJoint
						{
							uint8_t indices[4];
						};
						
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const TempJoint& tempJoint = *(const TempJoint*)(attributeData + i * stride);
							outModelInfo.vertexBoneIndices[vertexOffset + i].x = tempJoint.indices[0];
							outModelInfo.vertexBoneIndices[vertexOffset + i].y = tempJoint.indices[1];
							outModelInfo.vertexBoneIndices[vertexOffset + i].z = tempJoint.indices[2];
							outModelInfo.vertexBoneIndices[vertexOffset + i].w = tempJoint.indices[3];
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						struct TempJoint
						{
							uint16_t indices[4];
						};

						for (size_t i = 0; i != vertexCount; ++i)
						{
							const TempJoint& tempJoint = *(const TempJoint*)(attributeData + i * stride);
							outModelInfo.vertexBoneIndices[vertexOffset + i].x = tempJoint.indices[0];
							outModelInfo.vertexBoneIndices[vertexOffset + i].y = tempJoint.indices[1];
							outModelInfo.vertexBoneIndices[vertexOffset + i].z = tempJoint.indices[2];
							outModelInfo.vertexBoneIndices[vertexOffset + i].w = tempJoint.indices[3];
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							outModelInfo.vertexBoneIndices[vertexOffset + i] = *(const XMUINT4*)(attributeData + i * stride);
						}
					}
					else
					{
						LOG_ERROR("GLTFImporter::import(): Joint type is unsupported")
						return false;
					}
				}
				else if (attributeName == "WEIGHTS_0")
				{
					outModelInfo.vertexBoneWeights.resize(vertexOffset + vertexCount);
					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							outModelInfo.vertexBoneWeights[vertexOffset + i] = *(const XMFLOAT4*)(attributeData + i * stride);
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const uint8_t& x = attributeData[i * stride];
							const uint8_t& y = attributeData[i * stride + sizeof(uint8_t)];
							const uint8_t& z = attributeData[i * stride + sizeof(uint8_t) * 2];
							const uint8_t& w = attributeData[i * stride + sizeof(uint8_t) * 3];

							outModelInfo.vertexBoneWeights[vertexOffset + i] = XMFLOAT4(x / 255.0f, y / 255.0f, z / 255.0f, w / 255.0f);
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const uint16_t& x = attributeData[i * stride];
							const uint16_t& y = attributeData[i * stride + sizeof(uint16_t)];
							const uint16_t& z = attributeData[i * stride + sizeof(uint16_t) * 2];
							const uint16_t& w = attributeData[i * stride + sizeof(uint16_t) * 3];

							outModelInfo.vertexBoneWeights[vertexOffset + i] = XMFLOAT4(x / 65535.0f, y / 65535.0f, z / 65535.0f, w / 65535.0f);
						}
					}
					else
					{
						LOG_ERROR("GLTFImporter::import(): Weight type is unsupported")
						return false;
					}
				}
				else if (attributeName == "COLOR_0")
				{
					outModelInfo.vertexColors.resize(vertexOffset + vertexCount);
					if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && accessor.type == TINYGLTF_TYPE_VEC3)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const XMFLOAT3& color = *(const XMFLOAT3*)(attributeData + i * stride);
							outModelInfo.vertexColors[vertexOffset + i] = math::compress_color(color);
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT && accessor.type == TINYGLTF_TYPE_VEC4)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const XMFLOAT4& color = *(const XMFLOAT4*)(attributeData + i * stride);
							outModelInfo.vertexColors[vertexOffset + i] = math::compress_color(color);
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && accessor.type == TINYGLTF_TYPE_VEC3)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const uint8_t& r = attributeData[i * stride];
							const uint8_t& g = attributeData[i * stride + sizeof(uint8_t)];
							const uint8_t& b = attributeData[i * stride + sizeof(uint8_t) * 2];
							outModelInfo.vertexColors[vertexOffset + i] = math::compress_color(XMFLOAT3(r / 255.0f, g / 255.0f, b / 255.0f));
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE && accessor.type == TINYGLTF_TYPE_VEC4)
					{
						for (size_t i = 0; i != vertexCount; ++i)
                        {
                        	const uint8_t& r = attributeData[i * stride];
                        	const uint8_t& g = attributeData[i * stride + sizeof(uint8_t)];
                        	const uint8_t& b = attributeData[i * stride + sizeof(uint8_t) * 2];
                        	const uint8_t& a = attributeData[i * stride + sizeof(uint8_t) * 3];
                        	outModelInfo.vertexColors[vertexOffset + i] = math::compress_color(XMFLOAT4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f));
                        }
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT && accessor.type == TINYGLTF_TYPE_VEC3)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const uint16_t& r = attributeData[i * stride];
							const uint16_t& g = attributeData[i * stride + sizeof(uint16_t)];
							const uint16_t& b = attributeData[i * stride + sizeof(uint16_t) * 2];
							outModelInfo.vertexColors[vertexOffset + i] = math::compress_color(XMFLOAT3(r / 65535.0f, g / 65535.0f, b / 65535.0f));
						}
					}
					else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT && accessor.type == TINYGLTF_TYPE_VEC4)
					{
						for (size_t i = 0; i != vertexCount; ++i)
						{
							const uint16_t& r = attributeData[i * stride];
							const uint16_t& g = attributeData[i * stride + sizeof(uint16_t)];
							const uint16_t& b = attributeData[i * stride + sizeof(uint16_t) * 2];
							const uint16_t& a = attributeData[i * stride + sizeof(uint16_t) * 3];
							outModelInfo.vertexColors[vertexOffset + i] = math::compress_color(XMFLOAT4(r / 65535.0f, g / 65535.0f, b / 65535.0f, a / 65535.0f));
						}
					}
					else
					{
						LOG_ERROR("GLTFImporter::import(): Color type is unsupported")
						return false;
					}
				}
			}
		}
	}

	if (conversionContext.mergeMeshes)
		outModelInfos.push_back(mergedModelCreateInfo);

	return true;
}
