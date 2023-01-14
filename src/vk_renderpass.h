#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace vkutil
{
	class RenderPassBuilder
	{
		public:
			static RenderPassBuilder begin();
			
			RenderPassBuilder& add_color_attachment(
				VkFormat imageFormat,
				VkSampleCountFlagBits samples,
				VkImageLayout finalLayout,
				VkImageLayout refLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VkAttachmentDescriptionFlags flags = 0);

			RenderPassBuilder& add_depth_attachment(
				VkFormat imageFormat,
				VkSampleCountFlagBits samples,
				VkImageLayout finalLayout,
				VkImageLayout refLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VkAttachmentDescriptionFlags flags = 0);

			RenderPassBuilder& add_subpass(VkPipelineBindPoint pipelineType);
			
		    void build(VkDevice& device, VkRenderPass& renderPass, void* pNext = nullptr);
			
		private:
			struct AttachmentInfo
			{
				VkAttachmentDescription attachment;
				VkAttachmentReference attachmentReference;
			};

			bool _isLastDepthAttachment = false;
			std::vector<VkAttachmentDescription> _allAttachments;
			std::vector<VkAttachmentReference> _lastColorAttachemntsRef;
		    VkAttachmentReference _lastDepthAttachmentRef;

		    std::vector<VkSubpassDescription> _subpasses;
		    std::vector<VkSubpassDependency> _dependencies; 
	};
}
