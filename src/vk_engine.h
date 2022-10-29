// vulkan_guide.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "vk_types.h"
#include "vk_mesh.h"
#include "material_system.h"
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
	glm::vec4 color;
	glm::mat4 model;
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

	AllocatedBuffer _indirectCommandsBuffer;
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

		Attachment _depthImage;
		Attachment _offscrDepthImage;
		Attachment _offscrColorImage;

		VmaAllocator _allocator;

		DeletionQueue _mainDeletionQueue;	    

		std::vector<RenderObject> _renderables;

		std::unordered_map<std::string, Mesh> _meshes;
		std::vector<Texture> _loadedTextures;
		std::vector<Texture> _baseColorTextures;
		std::vector<Texture> _normalTextures;
		std::vector<Texture> _armTextures;
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
		void parse_prefabs();
};
