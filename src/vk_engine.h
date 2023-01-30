// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "user_interface.h"
#include "vk_types.h"
#include "vk_mesh.h"
#include "material_system.h"
#include "vk_scene.h"
#include "engine_actors.h"
#include <user_interface.h>
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
#define NODE_COUNT 20

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

struct GPUCameraData
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
	glm::vec4 position;
};

struct GPUSceneData
{
	uint32_t dirLightsAmount;
	uint32_t pointLightsAmount;
	uint32_t spotLightsAmount;
};

struct GPUObjectData
{
	//glm::vec4 color;
	glm::mat4 model;
	glm::vec4 originRad;	// bound
	glm::vec4 extents;	// bound
	
	uint32_t baseColorTexId;
	uint32_t normalTexId;
	uint32_t armTexId;
	uint32_t data;
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
	AllocatedBuffer _sceneDataBuffer;

	DescriptorAllocator _dynamicDescriptorAllocator;

	DeletionQueue _frameDeletionQueue;
};

// Data for the first pass in oit algorithm
struct TransparencyFirstPassData
{
	struct Node
	{
		glm::vec4 color;
		float depth;
		uint32_t next;
	};

	struct GeometryInfo
	{
		uint32_t count;
		uint32_t maxNodeCount;
	};

	VkFramebuffer framebuffer;
	VkRenderPass renderPass;
	Texture headIndex;
	AllocatedBufferT<Node> nodes;
	AllocatedBufferT<GeometryInfo> geometryInfo;

	// Not final approach. I have to think how to add it to the transparency pass.
	vkutil::PipelineBuilder pipelineBuilder;
	vkutil::ShaderPass* geometryPass;
	void setup_pipeline_builder();
	void create_shader_pass(VulkanEngine* engine);
	void cleanup(VulkanEngine* engine);
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

struct MeshObject
{
	Mesh* mesh{ nullptr };

	vkutil::Material* material{ nullptr };
	VkImageView baseColor{ VK_NULL_HANDLE };
	VkImageView normal{ VK_NULL_HANDLE };
	VkImageView arm{ VK_NULL_HANDLE };
	
	uint32_t customSortKey;
	glm::mat4 transformMatrix;

	uint32_t bDrawDeferredPass{ 0 };
	uint32_t bDrawForwardPass{ 1 };
	uint32_t bDrawShadowPass{ 1 };
	uint32_t bDrawPointShadowPass{ 1 };
	uint32_t bDrawSpotShadowPass{ 1 };
	uint32_t bDrawTransparencyPass{ 0 };
};

// structs for culling
struct CullData
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

struct CullParams
{
	bool occlusionCull;
	bool frustumCull;
	float drawDist;
	bool aabb;
	glm::vec3 aabbmin;
	glm::vec3 aabbmax;
	glm::mat4 projMatrix;
	glm::mat4 viewMatrix;
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

struct ThreadInfo
{
	VkCommandPool commandPool{ VK_NULL_HANDLE };
	std::vector<VkCommandBuffer> commandBuffers;
};

struct GBuffer
{
	Attachment albedo;
	Attachment normal;
	Attachment surface;		// Roughness and metallic
	Attachment depth;
	VkFramebuffer framebuffer;
	VkRenderPass renderPass;

	void cleanup(VulkanEngine* engine);
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
		VkRenderPass _mainOpaqueRenderPass;
		VkRenderPass _transparencyRenderPass;
		//VkRenderPass _transparencyRenderPass;
		std::vector<VkFramebuffer> _framebuffers;
		std::vector<VkFramebuffer> _mainOpaqueFramebuffers;
		VkFramebuffer _transparencyFramebuffer;
		//VkFramebuffer _transparencyFramebuffer;
		VkFramebuffer _dirLightShadowFramebuffer;

		DescriptorAllocator _descriptorAllocator;
		DescriptorLayoutCache _descriptorLayoutCache;

		vkutil::MaterialSystem _materialSystem;
		RenderScene _renderScene;
		ui::UserInterface _userInterface;

		Attachment _mainOpaqueDepthAttach;
		Attachment _mainOpaqueColorAttach;
		Attachment _transparencyColorAttach;
		Attachment _transparencyDepthAttach;
		VkSampler _mainOpaqueSampler;

		GBuffer _GBuffer;

		// Depth map data for culling
		Texture _depthPyramid;
		VkSampler _depthSampler;
		VkImageView _depthPyramideMips[16] = {};
		VkExtent2D _depthPyramidExtent { 1024 * 4, 1024 * 4 };
		int _depthPyramidWidth;
		int _depthPyramidHeight;
		int _depthPyramidLevels;

		//Texture _headIndex;
		TransparencyFirstPassData _transparencyData;

		std::vector<VkBufferMemoryBarrier> _beforeCullingBufferBarriers;
		std::vector<VkBufferMemoryBarrier> _afterCullingBufferBarriers;		// I should execute those barriers before drawing
		std::vector<VkImageMemoryBarrier> _afterShadowsBarriers;

		VkPipeline _depthReducePipeline;
		VkPipelineLayout _depthReduceLayout;
		VkPipeline _cullingPipeline;
		VkPipelineLayout _cullintPipelineLayout;

		VkDescriptorSetLayout _depthReduceDescriptorSetLayout;

		VmaAllocator _allocator;

		DeletionQueue _mainDeletionQueue;

		std::vector<MeshObject> _meshObjects;

		std::unordered_map<std::string, Mesh> _meshes;
		std::vector<Texture> _loadedTextures;
		std::vector<Texture> _baseColorTextures;
		std::vector<Texture> _normalTextures;
		std::vector<Texture> _armTextures;

		VkSampler _textureSampler;

		VkSampler _shadowMapSampler;

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
		uint32_t _numThreads;
		std::vector<ThreadInfo> _threadInfo;

		std::string _projectPath;

		//initializes everything in the engine
		void init();

		//shuts down the engine
		void cleanup();

		//draw loop
		void draw();

		//run main loop
		void run();
		
		AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

		// I have to make static create_attachment method for Attachment class
		void create_attachment(
			Attachment& attachment, 
			VkExtent3D imageExtent, 
			VkFormat format,
			VkImageUsageFlags usageFlags, 
			VkImageAspectFlags aspectFlags,
			uint32_t layerCount = 1);

	private:
		void init_vulkan();
		void init_engine_systems();
		void init_imgui();
		void init_swapchain();
		void init_commands();
		void init_renderpasses();
		void init_framebuffers();
		void init_shadow_maps();
		void init_first_pass_data_for_oit();
		void init_sync_structures();
		void init_output_quad();
		void init_buffers();
		void init_pipelines();
		void setup_compute_pipeline(vkutil::Shader* shader, VkPipeline& pipeline, VkPipelineLayout& pipelineLayout);
		size_t pad_uniform_buffer_size(size_t originalSize);

		void refresh_swapchain();

		void create_cube_map(
			Texture& texture,
			VkExtent3D imageExtent,
			VkFormat format,
			VkImageUsageFlags usageFlags,
			VkImageAspectFlags aspectFlags);

		void reallocate_buffer(AllocatedBuffer& buffer, size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
		{
			AllocatedBuffer newBuffer(this, size, usage, memoryUsage);

			if (buffer._buffer != VK_NULL_HANDLE)
			{
				get_current_frame()._frameDeletionQueue.push_function([=](){
					LOG_INFO("Before deleting buffer");
					vmaDestroyBuffer(_allocator, buffer._buffer, buffer._allocation);
					LOG_INFO("After deleting buffer");
				});
			}

			buffer = newBuffer;
		}

		void parse_prefabs();

		void refresh_multi_threads_command_buffers(uint32_t objectsPerThread);
		
		// methods for renderer
		void prepare_gpu_indirect_buffer(VkCommandBuffer cmd, RenderScene::MeshPass& meshPass);
		void prepare_data_for_drawing(VkCommandBuffer cmd);
		void prepare_per_frame_data(VkCommandBuffer cmd);
		void fill_renderable_objects();
		void culling(RenderScene::MeshPass& meshPass, VkCommandBuffer cmd, CullParams cullParams);
		void draw_deferred_pass(VkCommandBuffer cmd);
		void draw_forward_pass(VkCommandBuffer cmd, uint32_t swapchainImageIndex);
		void draw_tranparency_pass(VkCommandBuffer cmd);
		void draw_final_quad(VkCommandBuffer cmd, uint32_t swapchainImageIndex);
		void submit(VkCommandBuffer cmd, uint32_t swapchainImageIndex);
		void bake_shadow_maps(VkCommandBuffer cmd);
		void draw_shadow_pass(VkCommandBuffer cmd, vkutil::MeshpassType passType);
		void draw_objects_in_shadow_pass(VkCommandBuffer cmd, VkDescriptorSet globalDescriptorSet, RenderScene::MeshPass& meshPass, uint32_t id);
		void reallocate_light_buffer(LightType lightType);
		void depth_reduce(VkCommandBuffer cmd);
};
