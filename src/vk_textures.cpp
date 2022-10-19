#include <vk_textures.h>
#include <iostream>
#include <logger.h>

#include <vk_initializers.h>
#include <asset_loader.h>
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace vkutil
{
	AllocatedImage upload_image(VulkanEngine& engine, AllocatedBuffer& stagingBuffer, assets::TextureInfo& textureInfo, VkFormat imageFormat)
	{
		VkExtent3D imageExtent;
		imageExtent.width = textureInfo.pixelSize[0];
		imageExtent.height = textureInfo.pixelSize[1];
		imageExtent.depth = 1;

		VkImageCreateInfo dimg_info = vkinit::image_create_info(imageFormat, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, imageExtent);

		AllocatedImage newImage;

		VmaAllocationCreateInfo dimg_allocinfo{};
		dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(engine._allocator, &dimg_info, &dimg_allocinfo, &newImage._image, &newImage._allocation, nullptr);

		engine.immediate_submit([&](VkCommandBuffer cmd){
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier imageBarrier_toTransfer{};
			imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toTransfer.image = newImage._image;
			imageBarrier_toTransfer.subresourceRange = range;

			imageBarrier_toTransfer.srcAccessMask = 0;
			imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// I have to read about pipeline barrier
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

			VkBufferImageCopy copyRegion{};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageExtent = imageExtent;

			vkCmdCopyBufferToImage(cmd, stagingBuffer._buffer, newImage._image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
			imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// Convert image layout and access mask into best format to read from shader
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
		});

		engine._mainDeletionQueue.push_function([=](){
			vmaDestroyImage(engine._allocator, newImage._image, newImage._allocation);
		});

		return newImage;
	}

	bool load_image_from_file(VulkanEngine& engine, const char* file, AllocatedImage& outImage)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			LOG_ERROR("Failed to load texture {}", file);
			return false;
		}

		void* pixel_ptr = pixels;
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

		AllocatedBuffer stagingBuffer = engine.create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void* data;
		vmaMapMemory(engine._allocator, stagingBuffer._allocation, &data);
		memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));
		vmaUnmapMemory(engine._allocator, stagingBuffer._allocation);

		stbi_image_free(pixels);

		VkExtent3D imageExtent;
		imageExtent.width = static_cast<uint32_t>(texWidth);
		imageExtent.height = static_cast<uint32_t>(texHeight);
		imageExtent.depth = 1;

		VkImageCreateInfo dimg_info = vkinit::image_create_info(image_format, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, imageExtent);

		AllocatedImage newImage;

		VmaAllocationCreateInfo dimg_allocinfo{};
		dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(engine._allocator, &dimg_info, &dimg_allocinfo, &newImage._image, &newImage._allocation, nullptr);

		engine.immediate_submit([&](VkCommandBuffer cmd){
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;

			VkImageMemoryBarrier imageBarrier_toTransfer{};
			imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toTransfer.image = newImage._image;
			imageBarrier_toTransfer.subresourceRange = range;

			imageBarrier_toTransfer.srcAccessMask = 0;
			imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			// I have to read about pipeline barrier
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

			VkBufferImageCopy copyRegion{};
			copyRegion.bufferOffset = 0;
			copyRegion.bufferRowLength = 0;
			copyRegion.bufferImageHeight = 0;

			copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageSubresource.baseArrayLayer = 0;
			copyRegion.imageSubresource.layerCount = 1;
			copyRegion.imageSubresource.mipLevel = 0;
			copyRegion.imageExtent = imageExtent;

			vkCmdCopyBufferToImage(cmd, stagingBuffer._buffer, newImage._image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

			VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;
			imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			// Convert image layout and access mask into best format to read from shader
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
		});

		engine._mainDeletionQueue.push_function([=](){
			vmaDestroyImage(engine._allocator, newImage._image, newImage._allocation);
		});

		vmaDestroyBuffer(engine._allocator, stagingBuffer._buffer, stagingBuffer._allocation);

		LOG_SUCCESS("Texture loaded successfully {}", file);

		outImage = newImage;

		return true;
	}

	bool load_image_from_asset(VulkanEngine& engine, const char* filename, AllocatedImage& outImage)
	{
		assets::AssetFile file;
		bool loaded = assets::load_binaryFile(filename, file);

		if (!loaded)
		{
			LOG_ERROR("Error when loading image {}", filename);
			return false;
		}

		assets::TextureInfo textureInfo = assets::read_texture_info(&file);
		VkDeviceSize imageSize = textureInfo.textureSize;
		VkFormat imageFormat;
		switch (textureInfo.textureFormat)
		{
			case assets::TextureFormat::RGBA8:
				imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
				break;
			case assets::TextureFormat::RGB8:
				imageFormat = VK_FORMAT_R8G8B8_SNORM;
				break;
			default:
				return false;
		}

		AllocatedBuffer stagingBuffer = engine.create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

		void* data;
		
		vmaMapMemory(engine._allocator, stagingBuffer._allocation, &data);
		assets::unpack_texture(&textureInfo, file.binaryBlob.data(), file.binaryBlob.size(), (char*)data);
		vmaUnmapMemory(engine._allocator, stagingBuffer._allocation);
		
		outImage = upload_image(engine, stagingBuffer, textureInfo, imageFormat);
		vmaDestroyBuffer(engine._allocator, stagingBuffer._buffer, stagingBuffer._allocation);

		return true;
	}
}
