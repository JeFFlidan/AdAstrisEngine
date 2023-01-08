#include "vk_renderpass.h"
#include "logger.h"
#include <vulkan/vulkan_core.h>

namespace vkutil
{
	RenderPassBuilder RenderPassBuilder::begin()
	{
		RenderPassBuilder temp;
		return temp;
	}

	RenderPassBuilder& RenderPassBuilder::add_color_attachment(
		VkFormat imageFormat,
		VkSampleCountFlagBits samples,
		VkImageLayout finalLayout,
		VkImageLayout refLayout,
		VkAttachmentDescriptionFlags flags)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.flags = flags;
		colorAttachment.format = imageFormat;
		colorAttachment.samples = samples;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = finalLayout;

		_allAttachments.push_back(colorAttachment);

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = _allAttachments.size() - 1;
		colorAttachmentRef.layout = refLayout;

		_lastColorAttachemntsRef.push_back(colorAttachmentRef);

		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::add_depth_attachment(
		VkFormat imageFormat,
		VkSampleCountFlagBits samples,
		VkImageLayout finalLayout,
		VkImageLayout refLayout,
		VkAttachmentDescriptionFlags flags)
	{
		VkAttachmentDescription depthAttachment{};
		depthAttachment.flags = flags;
		depthAttachment.format = imageFormat;
		depthAttachment.samples = samples;
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = finalLayout;

		_allAttachments.push_back(depthAttachment);

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = _allAttachments.size() - 1;
		depthAttachmentRef.layout = refLayout;

		if (_isLastDepthAttachment)
		{
			LOG_ERROR("It already has one depth attachment but the older one attachemnt will be overwritten!");
		}
		else
		{
			_isLastDepthAttachment = true;
		}
		
		_lastDepthAttachmentRef = depthAttachmentRef;

		return *this;
	}

	RenderPassBuilder& RenderPassBuilder::add_subpass(VkPipelineBindPoint pipelineType)
	{
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = pipelineType;

		if (_lastColorAttachemntsRef.empty() && (!_isLastDepthAttachment))
			LOG_FATAL("Failed to create subpass because there are no attachments");
		
		if (!_lastColorAttachemntsRef.empty())
		{
			subpass.colorAttachmentCount = _lastColorAttachemntsRef.size();
			subpass.pColorAttachments = _lastColorAttachemntsRef.data();
		}
		if (_isLastDepthAttachment)
		{
			subpass.pDepthStencilAttachment = &_lastDepthAttachmentRef;
		}

		_subpasses.push_back(subpass);
		_lastColorAttachemntsRef.clear();

		if (!_lastColorAttachemntsRef.empty())
		{
			VkSubpassDependency colorDependency{};
			colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			colorDependency.dstSubpass = 0;
			colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			colorDependency.srcAccessMask = 0;
			colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			_dependencies.push_back(colorDependency);
		}

		if (_isLastDepthAttachment)
		{
			VkSubpassDependency depthDependency{};
			depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			depthDependency.dstSubpass = 0;
			depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			depthDependency.srcAccessMask = 0;
			depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
			depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			_dependencies.push_back(depthDependency);
			_isLastDepthAttachment = false;
		}

		return *this;
	}

	void RenderPassBuilder::build(VkDevice& device, VkRenderPass& renderPass)
	{
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = _allAttachments.size();
		renderPassInfo.pAttachments = _allAttachments.data();
		renderPassInfo.subpassCount = _subpasses.size();
		renderPassInfo.pSubpasses = _subpasses.data();
		renderPassInfo.dependencyCount = _dependencies.size();
		renderPassInfo.pDependencies = _dependencies.data();

		VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
		if (result != VK_SUCCESS)
			LOG_FATAL("Failed to create renderpass");
	}
}

