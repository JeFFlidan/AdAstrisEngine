// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "vk_types.h"
#include "vk_mesh.h"
#include "material_system.h"
#include "vk_scene.h"
#include <stdint.h>
#include <vk_descriptors.h>
#include <vk_camera.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <deque>
#include <functional>
#include <map>
#include <string>
#include <vulkan/vulkan_core.h>

//#define VK_RELEASE 1

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function)
	{
		deletors.push_back(function);
	}

	void flush()
	{
		for (auto it = deletors.rbegin(); it != deletors.rend(); ++it)
		{
			(*it)();
		}
		deletors.clear();
	}
};

struct MeshPushConstants
{
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct RenderObject
{
	Mesh* mesh;
	vkutil::Material* material;
	glm::mat4 transformMatrix;
	glm::vec4 color;
};

struct GPUCameraData
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};

struct GPUSceneData
{
	glm::vec4 fogColor;
	glm::vec4 forDistances;
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection;
	glm::vec4 sunlightColor;
};

struct GPUObjectData
{
	//glm::vec4 color;
	glm::mat4 model;
	glm::vec4 originRad;	// bound
	glm::vec4 extents;	// bound
};

struct UploadContext
{
	VkFence _uploadFence;
	VkCommandPool _commandPool;
	VkCommandBuffer _commandBuffer;
};

struct FrameData
{
	VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;

	VkCommandPool _commandPool;
	VkCommandBuffer _mainCommandBuffer;

	AllocatedBuffer _cameraBuffer;
	VkDescriptorSet _globalDescriptor;

	AllocatedBuffer _objectBuffer;
	VkDescriptorSet _objectDescriptor;

	VkDescriptorSet _texturesDescriptor{VK_NULL_HANDLE};
	VkDescriptorSet _outputQuadTexture{VK_NULL_HANDLE};

	AllocatedBuffer _indirectCommandsBuffer;

	DescriptorAllocator _dynamicDescriptorAllocator;

	DeletionQueue _frameDeletionQueue;
};

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoord;
};

struct Plane
{
	std::vector<Vertex> _vertices = {
		{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } },
		{ { -1.0f, -1.0f,0.0f }, { 0.0f, 0.0f } },
		{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
		{ { 1.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } },
		{ {  1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } },
		{ { -1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } }
	};

	AllocatedBuffer _vertexBuffer;

	static VertexInputDescription get_vertex_description();
};

struct IndirectBatch
{
	Mesh* mesh;
	vkutil::Material* material;
	uint32_t first;
	uint32_t count;
};

struct PrefabElementsNames
{
	std::string meshName;
	std::string materialName;
	//std::vector<std::string> textureNames;
};

struct MeshObject
{
	Mesh* mesh{ nullptr };

	vkutil::Material* material{ nullptr };
	uint32_t customSortKey;
	glm::mat4 transformMatrix;

	uint32_t bDrawForwardPass : 1;
	uint32_t bDrawShadowPass : 1;
};

// structs for culling
struct DrawCullData
{
	glm::mat4 view;
	float P00, P11, znear, zfar; // symmetric projection parameters
	float frustum[4];  // data for left/right/top/bottom frustum planes
	float lodBase, lodStep;  // lod distance i = base * pow(step, i)
	float pyramidWidth, pyramidHeight;  // depth pyramid size in texels

	uint32_t drawCount;

	int cullingEnabled;
	int lodEnabled;
	int occlusionEnabled;
	int distCull;
	int AABBcheck;
	float aabbmin_x;
	float aabbmin_y;
	float aabbmin_z;
	float aabbmax_x;
	float aabbmax_y;
	float aabbmax_z;	
};

struct ObjectData
{
	glm::mat4 model;
	glm::vec4 sphereBounds;
	glm::vec4 extents;
};

struct DrawCommand
{
	uint32_t indexCount;
	uint32_t instanceCount;
	uint32_t firstIndex;
	int vertexOffset;
	uint32_t firstInstance;
	uint32_t objectID;
	uint32_t batchID;
};

struct alignas(16) DepthReduceData
{
	glm::vec2 imageSize;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class VulkanEngine 
{
	public:
		VkInstance _instance;
		VkDebugUtilsMessengerEXT _debug_messenger;
		VkPhysicalDevice _chosenGPU;
		VkPhysicalDeviceProperties _gpuProperties;
		VkDevice _device;
		VkSurfaceKHR _surface;

		VkSwapchainKHR _swapchain;
		VkFormat _swapchainImageFormat;
		std::vector<VkImage> _swapchainImages;
		std::vector<VkImageView> _swapchainImageViews;	// I need the image view to interact with the image

		VkQueue _graphicsQueue;
		uint32_t _graphicsQueueFamily;

		VkRenderPass _renderPass;
		VkRenderPass _offscrRenderPass;
		std::vector<VkFramebuffer> _framebuffers;
		std::vector<VkFramebuffer> _offscrFramebuffers;

		VkDescriptorSetLayout _globalSetLayout;
		VkDescriptorSetLayout _objectSetLayout;
		VkDescriptorSetLayout _texturesSetLayout;
		VkDescriptorSetLayout _singleTextureSetLayout;
		VkDescriptorPool _descriptorPool;

		DescriptorAllocator _descriptorAllocator;
		DescriptorLayoutCache _descriptorLayoutCache;

		vkutil::MaterialSystem _materialSystem;
		RenderScene _renderScene;

		Attachment _depthImage;
		Attachment _offscrDepthImage;
		Attachment _offscrColorImage;
		VkSampler _offscrColorSampler;

		// Depth map data for culling
		AllocatedImage _depthPyramid;
		VkSampler _depthSampler;
		VkImageView _depthPyramideMips[16] = {};
		VkExtent2D _depthPyramidExtent { 1024 * 4, 1024 * 4 };
		int _depthPyramidWidth;
		int _depthPyramidHeight;
		int _depthPyramidLevels;

		VkPipeline _depthReducePipeline;
		VkPipelineLayout _depthReduceLayout;
		VkPipeline _cullingPipeline;
		VkPipelineLayout _cullintPipelineLayout;

		VkDescriptorSetLayout _depthReduceDescriptorSetLayout;

		VmaAllocator _allocator;

		DeletionQueue _mainDeletionQueue;

		std::vector<RenderObject> _renderables;

		std::unordered_map<std::string, Mesh> _meshes;
		std::vector<Texture> _loadedTextures;
		std::vector<Texture> _baseColorTextures;
		std::vector<Texture> _normalTextures;
		std::vector<Texture> _armTextures;

		std::vector<VkDescriptorImageInfo> _baseColorImageInfos;
		std::vector<VkDescriptorImageInfo> _normalImageInfos;
		VkSampler _textureSampler;
		std::vector<VkDescriptorImageInfo> _armImageInfos;

		std::vector<PrefabElementsNames> _meshAndMaterialNames;
		Plane _outputQuad;

		GPUSceneData _sceneParameters;
		AllocatedBuffer _sceneParameterBuffer;

		AllocatedBuffer _globalVertexBuffer;
		AllocatedBuffer _globalIndexBuffer;
		// Sizes don't contain byte size, only amount of vertices and indices
	    size_t _globalVertexBufferSize = 0;
	    size_t _globalIndexBufferSize = 0;

		bool _isInitialized{ false };
		int _frameNumber{0};        
		int _selectedShader{0};

		VkExtent2D _windowExtent{ 1700 , 900 };

		struct SDL_Window* _window{ nullptr };
		Camera camera{glm::vec3(0.0f, 0.0f, 3.0f)};

		FrameData _frames[FRAME_OVERLAP];
		FrameData& get_current_frame();

		UploadContext _uploadContext;
		void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

		std::string _projectPath;

		//initializes everything in the engine
		void init();

		//shuts down the engine
		void cleanup();

		//draw loop
		void draw();

		//run main loop
		void run();
		
		Mesh* get_mesh(const std::string& name);

		void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count);
		void draw_output_quad(VkCommandBuffer cmd);

		AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	private:
		void init_vulkan();
		void init_engine_systems();
		void init_imgui();
		void init_swapchain();
		void init_commands();
		void init_renderpasses();
		void init_framebuffers();
		void init_sync_structures();
		void init_scene();
		void init_descriptors();
		void init_pipelines();
		void setup_compute_pipeline(vkutil::Shader* shader, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);
		size_t pad_uniform_buffer_size(size_t originalSize);

		void create_attachment(
			Attachment& attachment, 
			VkExtent3D imageExtent, 
			VkFormat format, 
			VkImageUsageFlags usageFlags, 
			VkImageAspectFlags aspectFlags);

		std::vector<IndirectBatch> compact_draws(RenderObject* objects, int count);
		void bind_material(VkCommandBuffer cmd, vkutil::Material* mateial);
		void bind_mesh(VkCommandBuffer cmd, Mesh* mesh);
		void allocate_global_vertex_and_index_buffer(std::vector<Mesh> meshes);
		template<typename T>
		void reallocate_buffer(AllocatedBufferT<T>& buffer, size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		AllocatedBuffer reallocate_buffer(AllocatedBuffer buffer, size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		void parse_prefabs();

		// methods for renderer
		void fill_renderable_objects();
		void culling(RenderScene::MeshPass& meshPass, VkCommandBuffer cmd);
		void draw_forward_pass();
		void depth_reduce(VkCommandBuffer cmd);
		void prepare_data_for_drawing(VkCommandBuffer cmd);
};
