
#include "mainRenderer.h"

VKStr::Swapchain MainRenderer::initSwapChain(const VKStr::Device &device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkFormat depthStencilFormat,
	VkRenderPass renderPass, unsigned int bufferCount, VKStr::Swapchain * oldSwapchain)
{
	// ������������ ��������� (��������� �������� ������ ���������, �����������, �����-������� � ��)
	VKStr::Swapchain resultSwapchain;

	// ���������� � �����������
	VKStr::SurfaceInfo surfaceInfo = GetSurfaceInfo(device.physicalDevice, surface);

	// �������� ����������� ������� � ��������� ������������ �����������
	if (!surfaceInfo.IsSurfaceFormatSupported(surfaceFormat)) {
		throw std::runtime_error("Vulkan: Required surface format is not supported. (SwapChain)");
	}

	// �������� ����������� ������� �������
	if (!device.IsDepthFormatSupported(depthStencilFormat)) {
		throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. (SwapChain)");
	}

	// ���� ���-�� ������� ������
	if (bufferCount > 0) {
		// ��������� - �������� �� ������������ ������������� ���-�� ������� (� ������������ ��������������)
		if (bufferCount < surfaceInfo.capabilities.minImageCount || bufferCount > surfaceInfo.capabilities.maxImageCount) {
			std::string message = "Vulkan: Surface don't support " + std::to_string(bufferCount) + " images/buffers in swap-chain";
			throw std::runtime_error(message);
		}
	}
	// � ��������� ������ ���������� ����� ����������� ���-��
	else {
		bufferCount = (surfaceInfo.capabilities.minImageCount + 1) > surfaceInfo.capabilities.maxImageCount ? surfaceInfo.capabilities.maxImageCount : 
			(surfaceInfo.capabilities.minImageCount + 1);
	}

	// ����� ������ ������������� (FIFO_KHR �� ��������, ����� �������)
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	// ���� ������������� ���-�� ������� ������ ������� - ���� ����� ������� ����� ������� �����,
	// �� ����� ���� ���������� ��������� ��� ����������� ��� ������������
	if (bufferCount > 1) {
		for (const VkPresentModeKHR& availablePresentMode : surfaceInfo.presentModes) {
			//���� �������� - ������������ VK_PRESENT_MODE_MAILBOX_KHR (������������ �������������)
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				presentMode = availablePresentMode;
				break;
			}
		}
	}

	// ���������� � ����������� swap-chain
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = this->surface_;
	swapchainCreateInfo.minImageCount = bufferCount;                        // ����������� ���-�� �����������
	swapchainCreateInfo.imageFormat = surfaceFormat.format;					// ������ �����������
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;			// �������� ������������
	swapchainCreateInfo.imageExtent = surfaceInfo.capabilities.currentExtent;        // ���������� (����������) �����������
	swapchainCreateInfo.imageArrayLayers = 1;                               // ���� (1 ����, �� ����������������� ������)
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;   // ��� ������������ ����������� 

	// �������� ���������� � ������� � ���������� � �������������� ������ 
	resultSwapchain.imageFormat = swapchainCreateInfo.imageFormat;
	resultSwapchain.imageExtent = swapchainCreateInfo.imageExtent;

	// ���� ������ swap-chain ��� ������� - �������� ��� ���������� � ������� � ����������
	if (oldSwapchain != nullptr) {
		oldSwapchain->imageExtent = {};
		oldSwapchain->imageFormat = {};
	}

	// ������� ��������
	std::vector<unsigned int> queueFamilyIndices = {
		(unsigned int)device.queueFamilies.graphics,
		(unsigned int)device.queueFamilies.present
	};

	// ���� ��� ������ ������� � ������������� ������������ ������ ���������, ������ ������ � ������� (� ������ ������ � ������� �����������)
	// ������ ���� ������������� (������� ������������ VK_SHARING_MODE_CONCURRENT, ������ ��� ���� ���-�� �������� � �� �������)
	if (device.queueFamilies.graphics != device.queueFamilies.present) {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}
	// � ��������� ������ ���������� ����� VK_SHARING_MODE_EXCLUSIVE (� �������� �������� ������� ������ ���������)
	else {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	swapchainCreateInfo.preTransform = surfaceInfo.capabilities.currentTransform;                                        // �� ���������� �������������� ����������� 
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                                     // ���������� ����� ������ � ������� ������ � ������� (��� ����������)
	swapchainCreateInfo.presentMode = presentMode;                                                              // ��������� ������ ������������� (��� ��� ������� �����)
	swapchainCreateInfo.clipped = VK_TRUE;                                                                      // �� ��������� ������������� ������� ������ �������
	swapchainCreateInfo.oldSwapchain = (oldSwapchain != nullptr ? oldSwapchain->vkSwapchain : VK_NULL_HANDLE);  // ������ swap-chain 

	// �������� swap-chain (�������� ����� � �������������� ������)
	if (vkCreateSwapchainKHR(device.logicalDevice, &swapchainCreateInfo, nullptr, &(resultSwapchain.vkSwapchain)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateSwapchainKHR function. (SwapChain)");
	}

	// ����������� ����������� swap-chain, ���� �� ��� �������
	if (oldSwapchain != nullptr) {
		vkDestroySwapchainKHR(device.logicalDevice, oldSwapchain->vkSwapchain, nullptr);
		oldSwapchain->vkSwapchain = VK_NULL_HANDLE;
	}

	// �������� ������ ����������� swap-chain
	// ���-�� ����������� �� ���� ����� ���-�� ������� (�� ��� �������� bufferCount ��� �������� swap-chain)
	unsigned int swapChainImageCount = 0;
	vkGetSwapchainImagesKHR(device.logicalDevice, resultSwapchain.vkSwapchain, &swapChainImageCount, nullptr);
	resultSwapchain.images.resize(swapChainImageCount);
	vkGetSwapchainImagesKHR(device.logicalDevice, resultSwapchain.vkSwapchain, &swapChainImageCount, resultSwapchain.images.data());

	// ������ ���������� ������� image-views ��� ������� ����������� (������������ ��������� �������� ����������� ��������������� ������ �����������)
	// ���� ��� ������� ������ swap-chain - �������������� �������� ��� ��� image-views
	if (oldSwapchain != nullptr) {
		if (!oldSwapchain->imageViews.empty()) {
			for (VkImageView const &swapchainImageView : oldSwapchain->imageViews) {
				vkDestroyImageView(device.logicalDevice, swapchainImageView, nullptr);
			}
			oldSwapchain->imageViews.clear();
		}
	}

	// ��� ������� ����������� (image) swap-chain'� ������� ���� imageView ������
	for (unsigned int i = 0; i < resultSwapchain.images.size(); i++) {

		// ������������� imageView (���� �������� � ������)
		VkImageView swap�hainImageView;

		// ���������� ��� �������������
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = resultSwapchain.images[i];                       // ����� � ������������ swap-chain
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;                        // ��� ����������� (2� ��������)
		createInfo.format = surfaceFormat.format;							// ������ �����������
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;			// �� ���������
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		// ������� � �������� � ������
		if (vkCreateImageView(device.logicalDevice, &createInfo, nullptr, &swap�hainImageView) == VK_SUCCESS) {
			resultSwapchain.imageViews.push_back(swap�hainImageView);
		}
		else {
			throw std::runtime_error("Vulkan: Error in vkCreateImageView function. (SwapChain)");
		}
	}


	// ����� ������� (Z-�����)
	// ����� ����� ���� ���� ��� ���� �����-�������, ���� ��� �������/������� ����������� (� ������� �� ����������� swap-chain)
	// ��������� �� �� ���������� � ����������� (������ �� ���� ��������������� �� ������������ �� ������).

	// ���� ��� ������������ swap-chain (������� ������) ������� �������� ���������� �������� ������ �������
	if (oldSwapchain != nullptr) {
		oldSwapchain->depthStencil.Deinit(device.logicalDevice);
	}

	// ������� ����� �������-��������� (������� 2D-����������� � ��������� ��������)
	resultSwapchain.depthStencil = CreateImageSingle(
		device,
		VK_IMAGE_TYPE_2D,
		depthStencilFormat,
		{ surfaceInfo.capabilities.currentExtent.width, surfaceInfo.capabilities.currentExtent.height, 1 },
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_SAMPLE_COUNT_1_BIT,
		swapchainCreateInfo.imageSharingMode
		);


	// ������ ���������� ������� �����-������ ����������� � image-views �������� ����������� � ������ ������� (����������� �������)
	// ����� ���� ����� �������� ������ ������� swap-�hain (���� �� ��� �������)
	if (oldSwapchain != nullptr) {
		if (!oldSwapchain->framebuffers.empty()) {
			for (VkFramebuffer const &frameBuffer : oldSwapchain->framebuffers) {
				vkDestroyFramebuffer(device.logicalDevice, frameBuffer, nullptr);
			}
			oldSwapchain->framebuffers.clear();
		}
	}

	// �������� �� ���� image views � ������� �����-����� ��� �������
	for (unsigned int i = 0; i < resultSwapchain.imageViews.size(); i++) {

		// ����� ������ �����������
		VkFramebuffer framebuffer;

		std::vector<VkImageView> attachments(2);
		attachments[0] = resultSwapchain.imageViews[i];                             // �������� �������� (�� ������ �����-����� ����)		
		attachments[1] = resultSwapchain.depthStencil.vkImageView;                  // ����� ������� (���� �� ��� �����-������)

		// �������� ������ �����������
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;                          // �������� ������� �������
		framebufferInfo.attachmentCount = (uint32_t)attachments.size();   // ���-�� ��������
		framebufferInfo.pAttachments = attachments.data();                // ����� � image-views �������� ����������� swap-chain'�
		framebufferInfo.width = resultSwapchain.imageExtent.width;        // ���������� (������)
		framebufferInfo.height = resultSwapchain.imageExtent.height;      // ���������� (������)
		framebufferInfo.layers = 1;                                       // ���� ����

		// � ������ ��������� �������� - �������� � ������
		if (vkCreateFramebuffer(device.logicalDevice, &framebufferInfo, nullptr, &framebuffer) == VK_SUCCESS) {
			resultSwapchain.framebuffers.push_back(framebuffer);
		}
		else {
			throw std::runtime_error("Vulkan: Error in vkCreateFramebuffer function. (SwapChain)");
		}
	}

	tools::LogMessage("Vulkan: Swap-chain successfully initialized");

	return resultSwapchain;
}

VKStr::Image MainRenderer::CreateImageSingle(const VKStr::Device & device, VkImageType imageType, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage, VkImageAspectFlags subresourceRangeAspect, VkImageLayout initialLayout, VkMemoryPropertyFlags memoryProperties, VkImageTiling tiling, VkSampleCountFlagBits samples, VkSharingMode sharingMode)
{
	// �������������� ������ �����������
	VKStr::Image resultImage;
	resultImage.extent = extent;
	resultImage.format = format;

	// ������������ �����������
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = imageType;
	imageInfo.format = format;
	imageInfo.extent = extent;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = samples;//VK_SAMPLE_COUNT_4_BIT;  //!!
	imageInfo.tiling = tiling;
	imageInfo.sharingMode = sharingMode;
	imageInfo.usage = usage;
	imageInfo.initialLayout = initialLayout;

	// �������� �����������
	if (vkCreateImage(device.logicalDevice, &imageInfo, nullptr, &(resultImage.vkImage)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating image (SwapChain.cpp)");
	}

	// �������� ���������� � ������ � ������ ���������� �����������
	VkMemoryRequirements memReqs = {};
	vkGetImageMemoryRequirements(device.logicalDevice, resultImage.vkImage, &memReqs);

	// ������������ ��������� ������ (������ �� ����������)
	VkMemoryAllocateInfo memoryAllocInfo = {};
	memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocInfo.allocationSize = memReqs.size;
	memoryAllocInfo.memoryTypeIndex = GetMemoryTypeIndex(device.physicalDevice, memReqs.memoryTypeBits, memoryProperties);

	// ������������
	if (vkAllocateMemory(device.logicalDevice, &memoryAllocInfo, nullptr, &(resultImage.vkDeviceMemory)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while allocating memory for image (SwapChain.cpp)");
	}

	// ���������
	if (vkBindImageMemory(device.logicalDevice, resultImage.vkImage, resultImage.vkDeviceMemory, 0) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while binding memory to image (SwapChain.cpp)");
	}

	// ���������� ��� view-������� (� ����������� �� ���� �����������)
	VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_1D;
	if (imageInfo.imageType == VK_IMAGE_TYPE_2D) {
		viewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	else if (imageInfo.imageType == VK_IMAGE_TYPE_3D) {
		viewType = VK_IMAGE_VIEW_TYPE_3D;
	}

	// ������������ view-�������
	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = viewType;
	imageViewInfo.format = format;
	imageViewInfo.subresourceRange = {};
	imageViewInfo.subresourceRange.aspectMask = subresourceRangeAspect;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.image = resultImage.vkImage;

	// �������� view-�������
	if (vkCreateImageView(device.logicalDevice, &imageViewInfo, nullptr, &(resultImage.vkImageView)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating image view (SwapChain.cpp)");
	}

	return resultImage;
}

void MainRenderer::DeinitSwapchain(const VKStr::Device & device, VKStr::Swapchain * swapchain)
{
	// �������� �����-������
	if (!swapchain->framebuffers.empty()) {
		for (VkFramebuffer const &frameBuffer : swapchain->framebuffers) {
			vkDestroyFramebuffer(device.logicalDevice, frameBuffer, nullptr);
		}
		swapchain->framebuffers.clear();
	}

	// �������� image-views �������
	if (!swapchain->imageViews.empty()) {
		for (VkImageView const &imageView : swapchain->imageViews) {
			vkDestroyImageView(device.logicalDevice, imageView, nullptr);
		}
		swapchain->imageViews.clear();
	}

	// ������ ����������� Z-������
	swapchain->depthStencil.Deinit(device.logicalDevice);

	// �������� swap-chain
	if (swapchain->vkSwapchain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device.logicalDevice, swapchain->vkSwapchain, nullptr);
		swapchain->vkSwapchain = VK_NULL_HANDLE;
	}

	// �������� ���������� � ������
	swapchain->imageExtent = {};
	swapchain->imageFormat = {};

	tools::LogMessage("Vulkan: Swap-chain successfully deinitialized");
}

//----------------------------
//shadow
void MainRenderer::initShadowFrameBuffer(const VKStr::Device &device)
{
	this->shadowPass_.width = 2048;
	this->shadowPass_.height = 2048;



	// For shadow mapping we only need a depth attachment
	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.extent.width = shadowPass_.width;
	image.extent.height = shadowPass_.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.format = this->shadowPass_.depthFormat;																// Depth stencil attachment
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;		// We will sample directly from the depth attachment for the shadow mapping
	image.sharingMode = VK_SHARING_MODE_CONCURRENT;
	if (vkCreateImage(device.logicalDevice, &image, nullptr, &shadowPass_.depth.image) != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan: Error shadowImage create info");
	}

	VkMemoryAllocateInfo memAlloc{};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;
	vkGetImageMemoryRequirements(device.logicalDevice, shadowPass_.depth.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryTypeIndex(device.physicalDevice, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(device.logicalDevice, &memAlloc, nullptr, &shadowPass_.depth.memory) != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan: Error Shadow AllocateMemory");
	}
	if (vkBindImageMemory(device.logicalDevice, shadowPass_.depth.image, shadowPass_.depth.memory, 0) != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan: Error shadow bindMemory");
	}

	VkImageViewCreateInfo depthStencilView = {};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = this->shadowPass_.depthFormat;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;
	depthStencilView.image = shadowPass_.depth.image;

	if (vkCreateImageView(device.logicalDevice, &depthStencilView, nullptr, &shadowPass_.depth.view) != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan: Error shadow createImageView");
	}

	VkSamplerCreateInfo sampler = {};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 1.0f;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	if (vkCreateSampler(device.logicalDevice, &sampler, nullptr, &shadowPass_.depthSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan: Error Shadow createSampler");
	}

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.renderPass = shadowPass_.renderPass;
	fbufCreateInfo.attachmentCount = 1;
	fbufCreateInfo.pAttachments = &shadowPass_.depth.view;
	fbufCreateInfo.width = shadowPass_.width;
	fbufCreateInfo.height = shadowPass_.height;
	fbufCreateInfo.layers = 1;

	if (vkCreateFramebuffer(device.logicalDevice, &fbufCreateInfo, nullptr, &shadowPass_.frameBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Vulkan: Error shadow frameBuffer");
	}
	
}