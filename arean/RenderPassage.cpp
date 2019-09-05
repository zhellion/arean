
#include "mainRenderer.h"

VkRenderPass MainRenderer::initRenderPassage(const VKStr::Device &device, VkSurfaceKHR surface, VkFormat colorAttachmentFormat, VkFormat depthStencilFormat)
{
	// �������� ����������� ������� �������� (�����������)
	VKStr::SurfaceInfo surfaceInfo = GetSurfaceInfo(device.physicalDevice, surface);
	if (!surfaceInfo.IsFormatSupported(colorAttachmentFormat)) {
		throw std::runtime_error("Vulkan: Required surface format is not supported. (render-passage)");
	}

	// �������� ����������� ������� �������
	if (!device.IsDepthFormatSupported(depthStencilFormat)) {
		throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. (render-passage)");
	}
	//this->shadowPass_.depthFormat = depthStencilFormat;

	// ������ �������� ��������
	std::vector<VkAttachmentDescription> attachments;

	// ��������� ��������� �������� (�����������)
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = colorAttachmentFormat;                                       // ������ ����� ������ ��������������� ���� ��� ����� ����������� ��� �������� ����-�����
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  //VK_SAMPLE_COUNT_1_BIT;                                      // �� ������������ ����������� !!
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                                 // �� ����� ������ ������� - ������� ��������
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;                               // �� ����� ����� ������� - ������� �������� (��� ��������� �����������)
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                      // ��������� ��������� (������ �������) - �� ������������
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                    // ��������� ��������� (����� �������) - �� ������������
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                            // ���������� ������ � ������ 
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                        // ���������� ������ � �������� �������� ����� ��������� ����� ��������� ������� (��� �������������)
	attachments.push_back(colorAttachment);

	// �������� �������� ������� ��������� (z-�����)
	VkAttachmentDescription depthStencilAttachment = {};
	depthStencilAttachment.format = depthStencilFormat;
	depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;								   // !!
	depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           // �� ����� ������ ������� - ������� ��������
	depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                     // �� ����� ����� ������� - �� ����� �������� (������ �� ������������ ��� �����������, ����� �� �������)
	depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                // ��������� ��������� (������ �������) - �� ������������
	depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // ��������� ��������� (����� �������) - �� ������������
	depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // ���������� ������ � ������ 
	depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // ���������� ������ � �������� �������� ����� ��������� ����� ��������� ������� (�������-��������)
	attachments.push_back(depthStencilAttachment);


	// ������ ������ �� �������� ��������
	std::vector<VkAttachmentReference> colorAttachmentReferences = {
		{
			0,                                                       // ������ ������� ������� �������� 
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                 // ��������� ��� ���������� ����� VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		}
	};

	// ������ �� �������� �������-���������
	VkAttachmentReference depthStencilAttachemntReference = {
		1,                                                           // ������ ������� ������� �������� 
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL             // ��������� ��� ���������� ����� VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	// �������� ������������� ���-�������
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();  // ���-�� �������� ��������
	subpassDescription.pColorAttachments = colorAttachmentReferences.data();               // �������� �������� (�������� ��� ������)
	subpassDescription.pDepthStencilAttachment = &depthStencilAttachemntReference;         // �������-�������� (�� ������������)
	subpassDescription.inputAttachmentCount = 0;                                           // ���-�� ������� �������� (�� ������������)
	subpassDescription.pInputAttachments = nullptr;                                        // ������� �������� (�������� ��� ������, ����. ���� ��� ���� �������� � ����-��� ���-�������)
	subpassDescription.preserveAttachmentCount = 0;                                        // ���-�� �������� �������� (�� ������������)
	subpassDescription.pPreserveAttachments = nullptr;                                     // �������� �������� ����� ���� ������������ ��� �����-�������� ������������� � ������ ���-��������
	subpassDescription.pResolveAttachments = nullptr;                                      // Resolve-�������� (������� ��� ������ �� ������������)!!!

	// ��������� ������������ ���-��������

	std::vector<VkSubpassDependency> dependencies = {
		// ������ �����������, ������ ���������
		// ������� ���������� �� ��������������� (final) � ��������������� (initial)
		{
			VK_SUBPASS_EXTERNAL,                                                       // ����������� ���-������ - �������, �������
			0,                                                                         // ��������� ���-������ - ������ (� ������������)
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // ��������������� ��� ���������� ���-������ (�������) �������� �� ����� ���������� ���������
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // ������� ��������� (������) �������� �� ����� ������ �������� ���������� �� ���������
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
		// ������ �����������, ����� - ����� ���������
		// ������� ���������� �� ��������������� (initial) � ��������������� (final)
		{
			0,                                                                         // ��������� ���-������ - ������ (� ������������)
			VK_SUBPASS_EXTERNAL,                                                       // ��������� - �������, �������
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // ���������� ���-������ (������) �������� �� ����� ������ �������� ����������
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // ��������� ���-������ (�������) ����� �� ����� ���������� ���������
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
	};

	// ������� ������
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (unsigned int)attachments.size();              //���-�� �������� ��������
	renderPassInfo.pAttachments = attachments.data();                               //�������� ��������
	renderPassInfo.subpassCount = 1;                                                //���-�� ���-��������
	renderPassInfo.pSubpasses = &subpassDescription;                                //�������� ���-��������
	renderPassInfo.dependencyCount = (unsigned int)dependencies.size();             //���-�� ������������
	renderPassInfo.pDependencies = dependencies.data();                             //�����������

	//�������� �������
	VkRenderPass renderPass;
	if (vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Failed to create render passage!");
	}

	tools::LogMessage("Vulkan: Render passage successfully initialized");

	return renderPass;
}




void MainRenderer::DeinitRenderPassage(const VKStr::Device &device, VkRenderPass * renderPass)
{
	if (*renderPass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(device.logicalDevice, *renderPass, nullptr);
		*renderPass = VK_NULL_HANDLE;
		tools::LogMessage("Vulkan: Render passage successfully deinitialized");
	}
}

//-----------
//shadow
VkRenderPass MainRenderer::initShadowRenderPassage(const VKStr::Device &device)
{
	if (!device.IsDepthFormatSupported(VK_FORMAT_D16_UNORM)) {
		throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. (render-passage)");
	}

	this->shadowPass_.depthFormat = VK_FORMAT_D16_UNORM;

	// ������ �������� ��������
	std::vector<VkAttachmentDescription> attachments;


	// �������� �������� ������� ��������� (z-�����)
	VkAttachmentDescription depthStencilAttachment = {};
	depthStencilAttachment.format = shadowPass_.depthFormat;
	depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;								   
	depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           
	depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                // ��������� ��������� (������ �������) - �� ������������
	depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // ��������� ��������� (����� �������) - �� ������������
	depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // ���������� ������ � ������ 
	depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // ���������� ������ � �������� �������� ����� ��������� ����� ��������� ������� (�������-��������)
	attachments.push_back(depthStencilAttachment);


	// ������ �� �������� �������-���������
	VkAttachmentReference depthStencilAttachemntReference = {
		0,                                                           
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	// �������� ������������� ���-�������
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 0;  // ���-�� �������� ��������
	subpassDescription.pDepthStencilAttachment = &depthStencilAttachemntReference;         


	// ��������� ������������ ���-��������

	std::vector<VkSubpassDependency> dependencies = {
		// ������ �����������, ������ ���������
		// ������� ���������� �� ��������������� (final) � ��������������� (initial)
		{
			VK_SUBPASS_EXTERNAL,                                                       // ����������� ���-������ - �������, �������
			0,                                                                         // ��������� ���-������ - ������ (� ������������)
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,                                      // ��������������� ��� ���������� ���-������ (�������) �������� �� ����� ���������� ���������
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,                             // ������� ��������� (������) �������� �� ����� ������ �������� ���������� �� ���������
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
		// ������ �����������, ����� - ����� ���������
		// ������� ���������� �� ��������������� (initial) � ��������������� (final)
		{
			0,                                                                         // ��������� ���-������ - ������ (� ������������)
			VK_SUBPASS_EXTERNAL,                                                       // ��������� - �������, �������
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,                             // ���������� ���-������ (������) �������� �� ����� ������ �������� ����������
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,                                      // ��������� ���-������ (�������) ����� �� ����� ���������� ���������
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		}
	};

	// ������� ������
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachments.size();              //���-�� �������� ��������
	renderPassInfo.pAttachments = attachments.data();                               //�������� ��������
	renderPassInfo.subpassCount = 1;                                                //���-�� ���-��������
	renderPassInfo.pSubpasses = &subpassDescription;                                //�������� ���-��������
	renderPassInfo.dependencyCount = (unsigned int)dependencies.size();             //���-�� ������������
	renderPassInfo.pDependencies = dependencies.data();                             //�����������

	//�������� �������
	VkRenderPass renderPass;
	if (vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Failed to create render passage");
	}

	tools::LogMessage("Vulkan: Render passage successfully initialized");

	return renderPass;
}