#include "vk_types.h"
#include "engine_actors.h"
#include "vk_engine.h"
#include "logger.h"
#include <stdint.h>
#include <vulkan/vulkan_core.h>

AllocatedBuffer::AllocatedBuffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	allocate_buffer(engine, allocSize, usage, memoryUsage);
}

void AllocatedBuffer::create_buffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	allocate_buffer(engine, allocSize, usage, memoryUsage);
}

void AllocatedBuffer::copy_from(VulkanEngine* engine, void* srcBuffer, size_t sizeInBytes)
{
	void* data;
	vmaMapMemory(engine->_allocator, _allocation, &data);
	memcpy(data, srcBuffer, sizeInBytes);
	vmaUnmapMemory(engine->_allocator, _allocation);
	_bufferSize = sizeInBytes;
}

VkDescriptorBufferInfo AllocatedBuffer::get_info(bool isStorage, VkDeviceSize offset)
{
	VkDescriptorBufferInfo info;
	info.buffer = _buffer;
	info.offset = offset;
	info.range = (isStorage) ? VK_WHOLE_SIZE : _bufferSize;
	return info;
}

void AllocatedBuffer::copy_buffer_cmd(VulkanEngine* engine, VkCommandBuffer cmd , AllocatedBuffer* srcBuffer, AllocatedBuffer* dstBuffer, VkDeviceSize dstOffset, VkDeviceSize srcOffset)
{
	dstBuffer->_bufferSize = srcBuffer->_bufferSize;
	VkBufferCopy copy;
	copy.dstOffset = dstOffset;
	copy.srcOffset = srcOffset;
	copy.size = srcBuffer->_bufferSize;
	vkCmdCopyBuffer(cmd, srcBuffer->_buffer, dstBuffer->_buffer, 1, &copy);
}

void AllocatedBuffer::destroy_buffer(VulkanEngine* engine)
{
    vmaDestroyBuffer(engine->_allocator, _buffer, _allocation);
}

void AllocatedBuffer::allocate_buffer(VulkanEngine* engine, size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	_bufferSize = allocSize;

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;

	bufferInfo.size = allocSize;
	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo{};
	vmaallocInfo.usage = memoryUsage;

    VkResult res = vmaCreateBuffer(engine->_allocator, &bufferInfo, &vmaallocInfo, &_buffer, &_allocation, nullptr);
    
    if (res != VK_SUCCESS)
		LOG_ERROR("Failed to allocate buffer");
}

void Texture::destroy_texture(VulkanEngine* engine)
{
	vkDestroyImageView(engine->_device, imageView, nullptr);
	vmaDestroyImage(engine->_allocator, imageData.image, imageData.allocation);
}

void Attachment::destroy_attachment(VulkanEngine* engine)
{
	destroy_texture(engine);
}

void ShadowMap::destroy_shadow_map(VulkanEngine* engine, ShadowMap shadowMap)
{
	vkDestroyFramebuffer(engine->_device, shadowMap.framebuffer, nullptr);
	vkDestroyRenderPass(engine->_device, shadowMap.renderPass, nullptr);
	vkDestroyImageView(engine->_device, shadowMap.attachment.imageView, nullptr);
	vmaDestroyImage(engine->_allocator, shadowMap.attachment.imageData.image, shadowMap.attachment.imageData.allocation);
}

void ShadowMap::create_light_space_matrices(VulkanEngine* engine, ActorType lightType, uint32_t lightId, ShadowMap& shadowMap)
{
	switch (lightType)
	{
		case ActorType::DirectionalLight:
		{
			auto& dirLight = engine->_renderScene._dirLights[lightId];
			float nearPlane = 0.01f, farPlane = 600.0f;
	
			shadowMap.lightProjMat = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, nearPlane, farPlane);
			shadowMap.lightProjMat[1][1] *= -1;

			glm::vec3 position = glm::vec3(dirLight.direction) * ((-farPlane) * 0.75f);
			shadowMap.lightViewMat = glm::lookAt(position, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			dirLight.lightSpaceMat = shadowMap.lightProjMat * shadowMap.lightViewMat;
		
			break;
		}
		case ActorType::PointLight:
		{
			auto& pointLight = engine->_renderScene._pointLights[lightId];

			VkExtent3D extent = shadowMap.attachment.extent;
			float aspect = (float)extent.width / (float)extent.height;
			float nearPlane = 0.1f;
			float farPlane = 1024.0f;
			glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, nearPlane, farPlane);

			glm::vec3 pos = glm::vec3(pointLight.positionAndAttRadius);

			pointLight.lightSpaceMat[0] = shadowProj * glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
			pointLight.lightSpaceMat[1] = shadowProj * glm::lookAt(pos, pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
			pointLight.lightSpaceMat[2] = shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			pointLight.lightSpaceMat[3] = shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
			pointLight.lightSpaceMat[4] = shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
			pointLight.lightSpaceMat[5] = shadowProj * glm::lookAt(pos, pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

			pointLight.farPlane = farPlane;

			shadowMap.lightProjMat = shadowProj;
			shadowMap.lightViewMat = glm::lookAt(pos, pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
			
			break;
		}
		case ActorType::SpotLight:
		{
			auto& spotLight = engine->_renderScene._spotLights[lightId];

			VkExtent3D extent = shadowMap.attachment.extent;
			float aspect = (float)extent.width / (float)extent.height;
			float nearPlane = 0.1f;
			float farPlane = 1024.0f;

			glm::mat4 shadowProj = glm::perspective(glm::radians(110.0f), aspect, nearPlane, farPlane);
			shadowProj[1][1] *= -1;
			shadowMap.lightProjMat = shadowProj;
			glm::vec3 pos = glm::vec3(spotLight.positionAndDistance);

			float rx = spotLight.rotationAndInnerConeRadius.x;
			float ry = spotLight.rotationAndInnerConeRadius.y;
			float rz = spotLight.rotationAndInnerConeRadius.z;

			glm::vec3 dir;
			dir.x = glm::cos(ry) * glm::cos(rx);
			dir.y = glm::sin(rx);
			dir.z = glm::sin(ry) * glm::cos(rx);

			glm::mat4 viewMat = glm::lookAt(pos, pos + dir, glm::vec3(0.0f, 1.0f, 0.0f));
			shadowMap.lightViewMat = viewMat;

			spotLight.lightSpaceMat = shadowMap.lightProjMat * shadowMap.lightViewMat;
			
			break;
		}
	}
}

