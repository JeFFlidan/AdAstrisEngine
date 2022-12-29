#pragma once

#include "vk_types.h"
#include "vk_mesh.h"
#include "material_system.h"
#include "engine_actors.h"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <array>
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

// I need Handle because I would like to access elements using an index in a vector or other collection
template<typename T>
struct Handle
{
	uint32_t handle;
};

struct MeshObject;
struct Mesh;
struct GPUObjectData;

struct GPUIndirectObject
{
	VkDrawIndexedIndirectCommand command;
	uint32_t objectID;
	uint32_t batchID;
};

struct DrawMesh
{
	uint32_t firstVertex;
	uint32_t firstIndex;
	uint32_t indexCount;
	uint32_t vertexCount;
	bool isMerged;

	Mesh* original;
};

struct RenderableObject
{
	Handle<DrawMesh> meshID;
	Handle<vkutil::Material> material;

	uint32_t updateIndex;
	uint32_t customSortKey{0};

	vkutil::PerPassData<int32_t> passIndeces;

	glm::mat4 transformMatrix;
};

struct GPUInstance
{
	uint32_t objectID;
	uint32_t batchID;
};

class RenderScene
{
	public:
		struct PassMaterial
		{
			//VkDescriptorSet materialSet;
			vkutil::ShaderPass* shaderPass;
	
			bool operator==(const PassMaterial& other) const
			{
				return shaderPass == other.shaderPass;
			}		
		};
	
		struct PassObject
		{
			PassMaterial material;
			Handle<DrawMesh> meshID;
			Handle<RenderableObject> original;
			int32_t buildBatch;
			uint32_t customKey;
		};
	
		struct RenderBatch
		{
			Handle<PassObject> object;
			uint64_t sortKey;
	
			bool operator==(const RenderBatch& other) const
			{
				return object.handle == other.object.handle && sortKey == other.sortKey;
			}
		};
	
		struct IndirectBatch
		{
			Handle<DrawMesh> meshID;
			PassMaterial material;
			uint32_t first;
			uint32_t count;
		};
	
		struct MultiBatch
		{
			uint32_t first;
			uint32_t count;
		};
	
		struct MeshPass
		{
			public:
				std::vector<RenderScene::MultiBatch> multibatches;
				std::vector<RenderScene::IndirectBatch> batches;
				std::vector<RenderScene::RenderBatch> flatBatches;
				std::vector<Handle<RenderableObject>> unbatchedObjects;
		
				std::vector<PassObject> objects;	// All objects in certain MeshPass to render
		
				std::vector<Handle<PassObject>> reusableObjects;
				std::vector<Handle<PassObject>> objectToDelete;
		
				AllocatedBufferT<uint32_t> compactedInstanceBuffer;
				AllocatedBufferT<GPUInstance> passObjectsBuffer;
				AllocatedBufferT<GPUIndirectObject> drawIndirectBuffer;		// GPU indirect buffer after culling
				AllocatedBufferT<GPUIndirectObject> clearIndirectBuffer;	// CPU indirect buffer

				PassObject* get(Handle<PassObject> handle);
		
				vkutil::MeshpassType type;
		
				bool needsIndirectRefresh = true;
				bool needsInstanceRefresh = true;
		};

		bool bNeedsReloadingRenderables = true;
		
		std::vector<RenderableObject> _renderables;		// All objects in the scene to render
		std::vector<DrawMesh> _meshes;
		std::vector<vkutil::Material*> _materials;
		std::vector<Handle<RenderableObject>> _dirtyObjects;	// Objects which should be reupload to the GPU

		std::vector<VkDescriptorImageInfo> _baseColorInfos;
		std::vector<VkDescriptorImageInfo> _normalInfos;
		std::vector<VkDescriptorImageInfo> _armInfos;
		VkSampler _textureSampler;

		std::vector<actors::DirectionLight> _dirLights;
		std::vector<actors::PointLight> _pointLights;
		std::vector<actors::SpotLight> _spotLights;
		AllocatedBufferT<actors::DirectionLight> _dirLightsBuffer;
		AllocatedBufferT<actors::PointLight> _pointLightsBuffer;
		AllocatedBufferT<actors::SpotLight> _spotLightsBuffer;
	
		MeshPass _forwardPass;
		MeshPass _shadowPass;
		MeshPass _transparentForwardPass;
		
		AllocatedBuffer _globalVertexBuffer;
		AllocatedBuffer _globalIndexBuffer;
		AllocatedBufferT<GPUObjectData> _objectDataBuffer;		// GPUObjectdata
	
		size_t _globalVertexBufferSize = 0;
		size_t _globalIndexBufferSize = 0;
	
		std::unordered_map<Mesh*, Handle<DrawMesh>> _meshConvert;
		std::unordered_map<vkutil::Material*, Handle<vkutil::Material>> _materialConvert;

		void init();
		void cleanup(VulkanEngine* engine);

		Handle<RenderableObject> register_object(MeshObject* object);
		void register_object_batch(MeshObject* first, uint32_t count);

		void update_transform(Handle<RenderableObject> objectID, const glm::mat4 transformMatrix);
		void update_object(Handle<RenderableObject> objectID);

		void fill_object_data(GPUObjectData* data);
		void fill_indirect_array(GPUIndirectObject* data, MeshPass& pass);
		void fill_instances_array(GPUInstance* data, MeshPass& pass);
	
		void write_object(GPUObjectData* target, Handle<RenderableObject> objectID);
	
		void clear_dirty_objects();
	
		void build_batches();
	
		void merge_meshes(class VulkanEngine* engine);
	
		void refresh_pass(MeshPass* meshPass);
	
		void build_indirect_batches(MeshPass* pass, std::vector<IndirectBatch>& outBatches, std::vector<RenderScene::RenderBatch>& inobjects);
	
		RenderableObject* get_renderable_object(Handle<RenderableObject> objectID);
		DrawMesh* get_mesh(Handle<DrawMesh> meshID);
		vkutil::Material* get_material(Handle<vkutil::Material> materialID);
		MeshPass* get_mesh_pass(vkutil::MeshpassType type);
		
		Handle<DrawMesh> get_mesh_handle(Mesh* mesh);
		Handle<vkutil::Material> get_material_handle(vkutil::Material* material);

		private:
			VkDevice _device;
			void delete_batches(MeshPass* meshPass);
		    void fill_pass_objects(MeshPass* meshPass, std::vector<Handle<PassObject>>& passObjectsHandles);
			void fill_flat_batches(MeshPass* meshPass, std::vector<Handle<PassObject>>& passObjectsHandles);
			void fill_multi_batches(MeshPass* meshPass);
};

