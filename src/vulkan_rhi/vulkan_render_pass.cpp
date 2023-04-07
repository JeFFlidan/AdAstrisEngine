#include "vulkan_render_pass.h"
#include "profiler/logger.h"
#include "vulkan_common.h"

using namespace ad_astris;

vulkan::VulkanRenderPass::VulkanRenderPass(VulkanDevice* device, rhi::RenderPassInfo* passInfo) : _device(device)
{
	_extent.width = passInfo->renderTargets[0].target->texture->textureInfo.width;
	_extent.height = passInfo->renderTargets[0].target->texture->textureInfo.height;
	create_render_pass(passInfo);
	create_framebuffer(passInfo->renderTargets);
}

vulkan::VulkanRenderPass::~VulkanRenderPass()
{
	vkDestroyFramebuffer(_device->get_device(), _framebuffer, nullptr);
	vkDestroyRenderPass(_device->get_device(), _renderPass, nullptr);
}

VkRenderPassBeginInfo vulkan::VulkanRenderPass::get_begin_info()
{
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.renderPass = _renderPass;
	beginInfo.framebuffer = _framebuffer;
	beginInfo.renderArea.offset.x = 0;
	beginInfo.renderArea.offset.y = 0;
	beginInfo.renderArea.extent = _extent;
	std::vector<VkClearValue> clearValues;
	for (int i = 0; i != _colorAttachCount; ++i)
	{
		VkClearValue clearValue;
		clearValue.color = { {0.1f, 0.1f, 0.1f, 1.0f} };
		clearValues.push_back(clearValue);
	}
	if (_depthAttachCount)
	{
		VkClearValue clearValue;
		clearValue.depthStencil.depth = 1.0;
		clearValues.push_back(clearValue);
	}
	beginInfo.clearValueCount = clearValues.size();
	beginInfo.pClearValues = clearValues.data();
	return beginInfo;
}

// private
void vulkan::VulkanRenderPass::create_render_pass(rhi::RenderPassInfo* passInfo)
{
	std::vector<VkAttachmentDescription> attachDescriptions;
	std::vector<VkAttachmentReference> colorAttachRefs;
	VkAttachmentReference depthAttachRef;
	bool oneDepth = false;
	
	for (auto& target: passInfo->renderTargets)
	{
		rhi::TextureInfo texInfo = target.target->texture->textureInfo;
		VkAttachmentDescription attachInfo{};
		attachInfo.format = get_format(texInfo.format);
		attachInfo.samples = get_sample_count(texInfo.samplesCount);
		attachInfo.loadOp = get_attach_load_op(target.loadOp);
		attachInfo.storeOp = get_attach_store_op(target.storeOp);
		attachInfo.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachInfo.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachInfo.initialLayout = get_image_layout(target.initialLayout);
		attachInfo.finalLayout = get_image_layout(target.finalLayout);
		attachDescriptions.push_back(attachInfo);

		VkAttachmentReference attachRef{};
		attachRef.attachment = attachDescriptions.size() - 1;
		attachRef.layout = get_image_layout(target.renderPassLayout);

		if (target.type == rhi::RenderTargetType::DEPTH && !oneDepth)
		{
			depthAttachRef = attachRef;
			oneDepth = true;
			++_depthAttachCount;
		}
		else if (target.type == rhi::RenderTargetType::DEPTH && oneDepth)
		{
			LOG_WARNING("VulkanRHI::create_render_pass(): There are more than one depth attachment. Old one will be overwritten")
			depthAttachRef = attachRef;
		}
		else
		{
			colorAttachRefs.push_back(attachRef);
			++_colorAttachCount;
		}
	}

	VkSubpassDescription subpass{};
	if (passInfo->pipelineType == rhi::PipelineType::COMPUTE || passInfo->pipelineType == rhi::PipelineType::UNDEFINED)
	{
		LOG_ERROR("VulkanRHI::create_render_pass(): Invalid pipeline type")
		return;
	}

	subpass.pipelineBindPoint = get_pipeline_bind_point(passInfo->pipelineType);
	if (!colorAttachRefs.empty())
	{
		subpass.colorAttachmentCount = colorAttachRefs.size();
		subpass.pColorAttachments = colorAttachRefs.data();
	}
	if (oneDepth)
	{
		subpass.pDepthStencilAttachment = &depthAttachRef;
	}

	std::vector<VkSubpassDependency> dependencies;
	if (!colorAttachRefs.empty())
	{
		VkSubpassDependency colorDependency{};
		colorDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		colorDependency.dstSubpass = 0;
		colorDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorDependency.srcAccessMask = 0;
		colorDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		colorDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies.push_back(colorDependency);
	}
	if (oneDepth)
	{
		VkSubpassDependency depthDependency{};
		depthDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		depthDependency.dstSubpass = 0;
		depthDependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depthDependency.srcAccessMask = 0;
		depthDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		depthDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependencies.push_back(depthDependency);
	}

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachDescriptions.size();
	renderPassInfo.pAttachments = attachDescriptions.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	rhi::MultiviewInfo& multiviewInfo = passInfo->multiviewInfo;
	VkRenderPassMultiviewCreateInfoKHR renderPassMultiviewInfo{};
	
	if (multiviewInfo.isEnabled)
	{
		if (!multiviewInfo.viewCount)
		{
			LOG_ERROR("VulkanRHI::create_render_pass(): If multiview is used, view count can't be 0")
			return;
		}
		if (multiviewInfo.viewCount > _device->get_max_multiview_view_count())
		{
			LOG_ERROR("VulkanRHI::create_render_pass(): View count can't be greater than {}", _device->get_max_multiview_view_count())
			return;
		}

		const uint32_t viewMask = (1 << multiviewInfo.viewCount) - 1;
		const uint32_t correlationMask = (1 << multiviewInfo.viewCount) - 1;
		
		renderPassMultiviewInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO_KHR;
		renderPassMultiviewInfo.subpassCount = 1;
		renderPassMultiviewInfo.pViewMasks = &viewMask;
		renderPassMultiviewInfo.correlationMaskCount = 1;
		renderPassMultiviewInfo.pCorrelationMasks = &correlationMask;

		renderPassInfo.pNext = &renderPassMultiviewInfo;
	}
	
	VK_CHECK(vkCreateRenderPass(_device->get_device(), &renderPassInfo, nullptr, &_renderPass));
}

void vulkan::VulkanRenderPass::create_framebuffer(std::vector<rhi::RenderTarget>& renderTargets)
{
	std::vector<VkImageView> attachViews;
	for (auto& target : renderTargets)
	{
		VkImageView view = *static_cast<VkImageView*>(target.target->handle);
		attachViews.push_back(view);
	}
	
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.layers = 1;
	framebufferInfo.width = renderTargets[0].target->texture->textureInfo.width;
	framebufferInfo.height = renderTargets[0].target->texture->textureInfo.height;
	framebufferInfo.renderPass = _renderPass;
	framebufferInfo.pAttachments = attachViews.data();
	framebufferInfo.attachmentCount = attachViews.size();
	
	VK_CHECK(vkCreateFramebuffer(_device->get_device(), &framebufferInfo, nullptr, &_framebuffer));
}

