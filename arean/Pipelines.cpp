
#include "mainRenderer.h"

VkPipelineLayout MainRenderer::initPipelineLayout(const VKStr::Device &device, std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
{
	VkPipelineLayout resultLayout;

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
	pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pPipelineLayoutCreateInfo.pNext = nullptr;
	pPipelineLayoutCreateInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
	pPipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

	if (vkCreatePipelineLayout(device.logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &resultLayout) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating pipeline layout (initPipelineLoyout)");
	}

	tools::LogMessage("Vulkan: Pipeline layout successfully initialized");

	return resultLayout;
}

void MainRenderer::DeinitPipelineLayout(const VKStr::Device & device, VkPipelineLayout * pipelineLayout)
{
	if (device.logicalDevice != VK_NULL_HANDLE && pipelineLayout != nullptr && *pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(device.logicalDevice, *pipelineLayout, nullptr);
		*pipelineLayout = VK_NULL_HANDLE;

		tools::LogMessage("Vulkan: Pipeline layout successfully deinitialized");
	}
}

VkPipeline MainRenderer::initGraphicsPipeline(const VKStr::Device & device, VkPipelineLayout pipelineLayout, const VKStr::Swapchain & swapchain, VkRenderPass renderPass, std::string fShaderName, std::string vShaderName)
{
	VkPipeline resultPipelineHandle = VK_NULL_HANDLE;

	// ������������ �������� � ���������� ������� ������ (���������)
	std::vector<VkVertexInputBindingDescription> bindingDescription = GetVertexInputBindingDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = GetVertexInputAttributeDescriptions(0);

	// ������������ ������ ����� ��������� ������
	VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
	vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStage.vertexBindingDescriptionCount = (uint32_t)bindingDescription.size();
	vertexInputStage.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputStage.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();

	// �������� ����� "������" ������� ������
	// �������� ����� "��������" �������� � ����� �������������
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage = {};
	inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;					// ������ �������������
	inputAssemblyStage.primitiveRestartEnable = VK_FALSE;								// ������������ ���������� �� ������������

	// ������������� (���������) ����� ���������
	// ���������� 2 ������� - ��������� (��� ������ �������) � ����������� 
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			LoadSPIRVShader(vShaderName, device.logicalDevice),
			"main",
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			LoadSPIRVShader(fShaderName, device.logicalDevice),
			"main",
			nullptr
		}
	};

	// ��������� ������� ���������
	// ������� ����� �������� ����������� swap-chain, ������� � ���� ������� ����� �������� ����������� ���������
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain.imageExtent.width;
	viewport.height = (float)swapchain.imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// ��������� ������� ����������� (�� ��������, ������� ��������� � �������� ���������)
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.imageExtent;

	// ��������� ���� ������ � ������� ���������
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// ��������� ���� ������������
	VkPipelineRasterizationStateCreateInfo rasterizationStage = {};
	rasterizationStage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStage.depthClampEnable = VK_FALSE;                     // ��������� �� ������� � ������� ������ ������ ������������� (VK_TRUE ��� ���������������� �������)
	rasterizationStage.rasterizerDiscardEnable = VK_FALSE;              // ���������� ������������ ��������� - �� ����� (VK_TRUE ��� ���������������� �������)
	rasterizationStage.polygonMode = VK_POLYGON_MODE_FILL;              // ����������� ��������, ��� ����� � ����� ���� ������//https://vulkan.lunarg.com/doc/view/1.0.37.0/linux/vkspec.chunked/ch24s07.html#VkPolygonMode
	rasterizationStage.lineWidth = 1.0f;                                // ������ �����
	rasterizationStage.cullMode = VK_CULL_MODE_BACK_BIT;                // ��������� ������ (���������� ��, ��� ��������� �������)!!!
	rasterizationStage.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//VK_FRONT_FACE_CLOCKWISE;             // ������� ���������� ������ ��� ������� ����� - �� ������� �������!!!
	rasterizationStage.depthBiasEnable = VK_FALSE;                      // �������� �������� �������
	rasterizationStage.depthBiasConstantFactor = 0.0f;
	rasterizationStage.depthBiasClamp = 0.0f;
	rasterizationStage.depthBiasSlopeFactor = 0.0f;

	// ��������� ���� z-����� (����� �������)
	// ������������ ���� �������, ������������ ��������� "������ ��� �����"
	VkPipelineDepthStencilStateCreateInfo depthStencilStage = {};
	depthStencilStage.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStage.depthTestEnable = VK_TRUE;
	depthStencilStage.depthWriteEnable = VK_TRUE;
	depthStencilStage.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilStage.depthBoundsTestEnable = VK_FALSE;
	depthStencilStage.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilStage.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStage.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilStage.stencilTestEnable = VK_FALSE;
	depthStencilStage.front = depthStencilStage.back;

	// ��������� ���� ��������������� (����������� ���������� �������)
	// ������������� - ��������� �������������� �������� ����� � ������ (������), ��� ����� ������� ���������� ��� �����
	VkPipelineMultisampleStateCreateInfo multisamplingStage = {};
	multisamplingStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingStage.sampleShadingEnable = VK_TRUE;
	multisamplingStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  //VK_SAMPLE_COUNT_1_BIT;//����������� 1 �������
	multisamplingStage.minSampleShading = 1.0f;
	multisamplingStage.pSampleMask = nullptr;
	multisamplingStage.alphaToCoverageEnable = VK_FALSE;
	multisamplingStage.alphaToOneEnable = VK_FALSE;

	// ���� ���������� ����� (��� ������� �����-������)
	// �� ���� ����� ����� ��������� ��� ������ ������������, ��������, �������������� �������
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// ���������� ��������� ����������
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachment;

	// ���������� ������������� ������������ ���������
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)shaderStages.size();    // ���-�� ��������� ������
	pipelineInfo.pStages = shaderStages.data();                 // ��������� ����� (�� �����������)
	pipelineInfo.pVertexInputState = &vertexInputStage;         // ��������� ����� ����� ��������� ������
	pipelineInfo.pInputAssemblyState = &inputAssemblyStage;     // ��������� ����� ������ ���������� �� ���������� ������
	pipelineInfo.pViewportState = &viewportState;               // ��������� ������� ���������
	pipelineInfo.pRasterizationState = &rasterizationStage;     // ��������� ����� ������������
	pipelineInfo.pDepthStencilState = &depthStencilStage;       // ��������� ����� z-�����
	pipelineInfo.pMultisampleState = &multisamplingStage;       // ��������� ����� ���������������
	pipelineInfo.pColorBlendState = &colorBlendState;           // ��������� ����� ���������� ������
	pipelineInfo.layout = pipelineLayout;                       // ���������� ���������
	pipelineInfo.renderPass = renderPass;                       // ��������� �������� � ��������������� �������� ����������
	pipelineInfo.subpass = 0;                                   // ��������� � ���-�������� (������ ���-������)

	// �������� ������������ ���������
	if (vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resultPipelineHandle) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating pipeline");
	}

	tools::LogMessage("Vulkan: Pipeline sucessfully initialized");

	// ��������� ������ ������ �� ����� ����� �������� ���������
	for (VkPipelineShaderStageCreateInfo &shaderStageInfo : shaderStages) {
		vkDestroyShaderModule(device.logicalDevice, shaderStageInfo.module, nullptr);
	}

	return resultPipelineHandle;
}
//����� �������� �������, ������ � �������� ������������ �������������, ������������ ��� ���������.
VkPipeline MainRenderer::initSBGraphicsPipeline(const VKStr::Device & device, VkPipelineLayout pipelineLayout, const VKStr::Swapchain & swapchain, VkRenderPass renderPass, std::string fShaderName, std::string vShaderName)
{
	VkPipeline resultPipelineHandle = VK_NULL_HANDLE;

	// ������������ �������� � ���������� ������� ������ (���������)
	std::vector<VkVertexInputBindingDescription> bindingDescription = GetVertexInputBindingDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = GetVertexInputAttributeDescriptions(0);

	// ������������ ������ ����� ��������� ������
	VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
	vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStage.vertexBindingDescriptionCount = (uint32_t)bindingDescription.size();
	vertexInputStage.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputStage.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();

	// �������� ����� "������" ������� ������
	// �������� ����� "��������" �������� � ����� �������������
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage = {};
	inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;					// ������ �������������
	inputAssemblyStage.primitiveRestartEnable = VK_FALSE;								// ������������ ���������� �� ������������

	// ������������� (���������) ����� ���������
	// ���������� 2 ������� - ��������� (��� ������ �������) � ����������� 
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			LoadSPIRVShader(vShaderName, device.logicalDevice),
			"main",
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			LoadSPIRVShader(fShaderName, device.logicalDevice),
			"main",
			nullptr
		}
	};

	// ��������� ������� ���������
	// ������� ����� �������� ����������� swap-chain, ������� � ���� ������� ����� �������� ����������� ���������
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain.imageExtent.width;
	viewport.height = (float)swapchain.imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// ��������� ������� ����������� (�� ��������, ������� ��������� � �������� ���������)
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.imageExtent;

	// ��������� ���� ������ � ������� ���������
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// ��������� ���� ������������
	VkPipelineRasterizationStateCreateInfo rasterizationStage = {};
	rasterizationStage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStage.depthClampEnable = VK_FALSE;                     // ��������� �� ������� � ������� ������ ������ ������������� (VK_TRUE ��� ���������������� �������)
	rasterizationStage.rasterizerDiscardEnable = VK_FALSE;              // ���������� ������������ ��������� - �� ����� (VK_TRUE ��� ���������������� �������)
	rasterizationStage.polygonMode = VK_POLYGON_MODE_FILL;              // ����������� ��������, ��� ����� � ����� ���� ������//https://vulkan.lunarg.com/doc/view/1.0.37.0/linux/vkspec.chunked/ch24s07.html#VkPolygonMode
	rasterizationStage.lineWidth = 1.0f;                                // ������ �����
	rasterizationStage.cullMode = VK_CULL_MODE_FRONT_BIT;                // ��������� ������ (���������� ��, ��� ��������� ���������)!!!
	rasterizationStage.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//VK_FRONT_FACE_CLOCKWISE;             // ������� ���������� ������ ��� ������� ����� - �� ������� �������!!!
	rasterizationStage.depthBiasEnable = VK_FALSE;                      // �������� �������� �������
	rasterizationStage.depthBiasConstantFactor = 0.0f;
	rasterizationStage.depthBiasClamp = 0.0f;
	rasterizationStage.depthBiasSlopeFactor = 0.0f;

	// ��������� ���� z-����� (����� �������)
	// ������������ ���� �������, ������������ ��������� "������ ��� �����"
	VkPipelineDepthStencilStateCreateInfo depthStencilStage = {};
	depthStencilStage.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStage.depthTestEnable = VK_TRUE;
	depthStencilStage.depthWriteEnable = VK_TRUE;
	depthStencilStage.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilStage.depthBoundsTestEnable = VK_FALSE;
	depthStencilStage.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilStage.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStage.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilStage.stencilTestEnable = VK_FALSE;
	depthStencilStage.front = depthStencilStage.back;

	// ��������� ���� ��������������� (����������� ���������� �������)
	// ������������� - ��������� �������������� �������� ����� � ������ (������), ��� ����� ������� ���������� ��� �����
	VkPipelineMultisampleStateCreateInfo multisamplingStage = {};
	multisamplingStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingStage.sampleShadingEnable = VK_TRUE;
	multisamplingStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  //VK_SAMPLE_COUNT_1_BIT;//����������� 1 �������
	multisamplingStage.minSampleShading = 1.0f;
	multisamplingStage.pSampleMask = nullptr;
	multisamplingStage.alphaToCoverageEnable = VK_FALSE;
	multisamplingStage.alphaToOneEnable = VK_FALSE;

	// ���� ���������� ����� (��� ������� �����-������)
	// �� ���� ����� ����� ��������� ��� ������ ������������, ��������, �������������� �������
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// ���������� ��������� ����������
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachment;

	// ���������� ������������� ������������ ���������
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)shaderStages.size();    // ���-�� ��������� ������
	pipelineInfo.pStages = shaderStages.data();                 // ��������� ����� (�� �����������)
	pipelineInfo.pVertexInputState = &vertexInputStage;         // ��������� ����� ����� ��������� ������
	pipelineInfo.pInputAssemblyState = &inputAssemblyStage;     // ��������� ����� ������ ���������� �� ���������� ������
	pipelineInfo.pViewportState = &viewportState;               // ��������� ������� ���������
	pipelineInfo.pRasterizationState = &rasterizationStage;     // ��������� ����� ������������
	pipelineInfo.pDepthStencilState = &depthStencilStage;       // ��������� ����� z-�����
	pipelineInfo.pMultisampleState = &multisamplingStage;       // ��������� ����� ���������������
	pipelineInfo.pColorBlendState = &colorBlendState;           // ��������� ����� ���������� ������
	pipelineInfo.layout = pipelineLayout;                       // ���������� ���������
	pipelineInfo.renderPass = renderPass;                       // ��������� �������� � ��������������� �������� ����������
	pipelineInfo.subpass = 0;                                   // ��������� � ���-�������� (������ ���-������)

	// �������� ������������ ���������
	if (vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resultPipelineHandle) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating pipeline");
	}

	tools::LogMessage("Vulkan: Pipeline sucessfully initialized");

	// ��������� ������ ������ �� ����� ����� �������� ���������
	for (VkPipelineShaderStageCreateInfo &shaderStageInfo : shaderStages) {
		vkDestroyShaderModule(device.logicalDevice, shaderStageInfo.module, nullptr);
	}

	return resultPipelineHandle;
}

void MainRenderer::DeinitGraphicsPipeline(const VKStr::Device & device, VkPipeline * pipeline)
{
	if (device.logicalDevice != VK_NULL_HANDLE && pipeline != nullptr && *pipeline != VK_NULL_HANDLE) {
		vkDestroyPipeline(device.logicalDevice, *pipeline, nullptr);
		*pipeline = VK_NULL_HANDLE;
		tools::LogMessage("Vulkan: Pipeline sucessfully deinitialized");
	}
}

std::vector<VkVertexInputBindingDescription> MainRenderer::GetVertexInputBindingDescriptions(unsigned int bindingIndex)
{
	return
	{
		{
			bindingIndex,                   // ������ �������� ��������� �������
			sizeof(VKStr::Vertex),      // ����������� ����
			VK_VERTEX_INPUT_RATE_VERTEX     // ������� �������� � ���������
		}
	};
}

std::vector<VkVertexInputAttributeDescription> MainRenderer::GetVertexInputAttributeDescriptions(unsigned int bindingIndex)
{
	return
	{
		{
			0,                                      // ������ ��������� (location � �������)
			bindingIndex,                           // ������ �������� ��������� �������
			VK_FORMAT_R32G32B32_SFLOAT,             // ��� ��������� (������������� vec3 � �������)
			offsetof(VKStr::Vertex, position)       // C���� � ���������
		},
		{
			1,
			bindingIndex,
			VK_FORMAT_R32G32B32_SFLOAT,
			offsetof(VKStr::Vertex, color)
		},
		{
			2,
			bindingIndex,
			VK_FORMAT_R32G32_SFLOAT,               // ��� ��������� (������������� vec2 � �������)
			offsetof(VKStr::Vertex, texCoord)
		},
		{
			3,
			bindingIndex,
			VK_FORMAT_R32_UINT,                    // ��� ��������� (������������� uint � �������)
			offsetof(VKStr::Vertex, textureUsed)
		},
		{
			4,
			bindingIndex,
			VK_FORMAT_R32G32B32_SFLOAT,                    
			offsetof(VKStr::Vertex, normals)
		},
	};
}

VkShaderModule MainRenderer::LoadSPIRVShader(std::string filename, VkDevice logicalDevice)
{
	// ������
	size_t shaderSize;

	// ���������� ����� (��� �������)
	char* shaderCode = nullptr;

	// ��������� ��� �������
	bool loaded = tools::LoadBytesFromFile(tools::DirProg() + "/../shaders/" + filename, &shaderCode, &shaderSize);

	// ���� �� ������� ��������� ��� ���� ����
	if (!loaded || shaderSize == 0) {
		std::string msg = "Vulkan: Error while loading shader code from file " + filename;
		throw std::runtime_error(msg);
	}

	// ������������ ���������� ������
	VkShaderModuleCreateInfo moduleCreateInfo{};
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = shaderSize;
	moduleCreateInfo.pCode = (unsigned int*)shaderCode;

	// ������� ��������� ������
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice, &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		std::string msg = "Vulkan: Error whiler creating shader module from file " + filename;
		throw std::runtime_error(msg);
	}

	// ������� ��� ������� (������� ��� ����� �� �����, ��������� �� ��� ������� � ��������� ������)
	delete[] shaderCode;

	
	return shaderModule;
}
//--------------------------------------------------------------------------------
//shadowmap
VkPipeline MainRenderer::initShadowPipeline(const VKStr::Device & device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, std::string vShaderName)
{
	VkPipeline resultPipelineHandle = VK_NULL_HANDLE;


	// ������������ �������� � ���������� ������� ������ (���������)
	std::vector<VkVertexInputBindingDescription> bindingDescription = GetVertexInputBindingDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = GetVertexInputAttributeDescriptions(0);

	// ������������ ������ ����� ��������� ������
	VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
	vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStage.vertexBindingDescriptionCount = (uint32_t)bindingDescription.size();
	vertexInputStage.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputStage.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();

	// �������� ����� "������" ������� ������
	// �������� ����� "��������" �������� � ����� �������������
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage = {};
	inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;					// ������ �������������
	inputAssemblyStage.primitiveRestartEnable = VK_FALSE;								// ������������ ���������� �� ������������

	// ������������� (���������) ����� ���������
	// ���������� 1 ������ (���������)
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = {
		{
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			nullptr,
			0,
			VK_SHADER_STAGE_VERTEX_BIT,
			LoadSPIRVShader(vShaderName, device.logicalDevice),
			"main",
			nullptr
		}
	};

	// ��������� ������� ���������
	// ������� ����� �������� ����������� swap-chain, ������� � ���� ������� ����� �������� ����������� ���������
	VkViewport viewport = {};
	viewport.width = (float)shadowPass_.width;
	viewport.height = (float)shadowPass_.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// ��������� ������� ����������� (�� ��������, ������� ��������� � �������� ���������)
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = shadowPass_.width;
	scissor.extent.height = shadowPass_.height;

	// ��������� ���� ������ � ������� ���������
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	viewportState.flags = 0;

	// ��������� ���� ������������
	VkPipelineRasterizationStateCreateInfo rasterizationStage = {};
	rasterizationStage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStage.depthClampEnable = VK_FALSE;                     // ��������� �� ������� � ������� ������ ������ ������������� (VK_TRUE ��� ���������������� �������)
	//rasterizationStage.rasterizerDiscardEnable = VK_FALSE;              // ���������� ������������ ��������� - �� ����� (VK_TRUE ��� ���������������� �������)
	rasterizationStage.polygonMode = VK_POLYGON_MODE_FILL;              // ����������� ��������, ��� ����� � ����� ���� ������//https://vulkan.lunarg.com/doc/view/1.0.37.0/linux/vkspec.chunked/ch24s07.html#VkPolygonMode
	rasterizationStage.lineWidth = 1.0f;                                // ������ �����
	rasterizationStage.cullMode = VK_CULL_MODE_BACK_BIT;                // ��������� ������ (���������� ��, ��� ��������� �������)!!!
	rasterizationStage.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//VK_FRONT_FACE_CLOCKWISE;             // ������� ���������� ������ ��� ������� ����� - �� ������� �������!!!
	rasterizationStage.depthBiasEnable = VK_TRUE;                      // �������� �������� �������
	rasterizationStage.depthBiasConstantFactor = 1.25f;
	rasterizationStage.depthBiasClamp = 0.0f;
	rasterizationStage.depthBiasSlopeFactor = 1.75f;

	// ��������� ���� z-����� (����� �������)
	// ������������ ���� �������, ������������ ��������� "������ ��� �����"
	VkPipelineDepthStencilStateCreateInfo depthStencilStage = {};
	depthStencilStage.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStage.depthTestEnable = VK_TRUE;
	depthStencilStage.depthWriteEnable = VK_TRUE;
	depthStencilStage.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	//depthStencilStage.depthBoundsTestEnable = VK_FALSE;
	//depthStencilStage.back.failOp = VK_STENCIL_OP_KEEP;
	//depthStencilStage.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilStage.back.compareOp = VK_COMPARE_OP_ALWAYS;
	//depthStencilStage.stencilTestEnable = VK_FALSE;
	depthStencilStage.front = depthStencilStage.back;

	// ��������� ���� ��������������� (����������� ���������� �������)
	// ������������� - ��������� �������������� �������� ����� � ������ (������), ��� ����� ������� ���������� ��� �����
	VkPipelineMultisampleStateCreateInfo multisamplingStage = {};
	multisamplingStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//multisamplingStage.sampleShadingEnable = VK_TRUE;
	multisamplingStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  //VK_SAMPLE_COUNT_1_BIT;//����������� 1 �������
	//multisamplingStage.minSampleShading = 1.0f;
	//multisamplingStage.pSampleMask = nullptr;
	//multisamplingStage.alphaToCoverageEnable = VK_FALSE;
	//multisamplingStage.alphaToOneEnable = VK_FALSE;
	multisamplingStage.flags = 0;

	// ���� ���������� ����� (��� ������� �����-������)
	// �� ���� ����� ����� ��������� ��� ������ ������������, ��������, �������������� �������
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = 0xf;
	colorBlendAttachment.blendEnable = VK_FALSE;
	//colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// ���������� ��������� ����������
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	//colorBlendState.logicOpEnable = VK_FALSE;
	//colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 0;
	colorBlendState.pAttachments = &colorBlendAttachment;

	std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

	VkPipelineDynamicStateCreateInfo dynamicStateCI = {};
	dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
	dynamicStateCI.dynamicStateCount = dynamicStateEnables.size();
	dynamicStateCI.flags = 0;

	// ���������� ������������� ������������ ���������
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)shaderStages.size();    // ���-�� ��������� ������
	pipelineInfo.pStages = shaderStages.data();                 // ��������� ����� (�� �����������)
	pipelineInfo.pVertexInputState = &vertexInputStage;         // ��������� ����� ����� ��������� ������
	pipelineInfo.pInputAssemblyState = &inputAssemblyStage;     // ��������� ����� ������ ���������� �� ���������� ������
	pipelineInfo.pViewportState = &viewportState;               // ��������� ������� ���������
	pipelineInfo.pRasterizationState = &rasterizationStage;     // ��������� ����� ������������
	pipelineInfo.pDepthStencilState = &depthStencilStage;       // ��������� ����� z-�����
	pipelineInfo.pMultisampleState = &multisamplingStage;       // ��������� ����� ���������������
	pipelineInfo.pColorBlendState = &colorBlendState;           // ��������� ����� ���������� ������
	pipelineInfo.pDynamicState = &dynamicStateCI;
	pipelineInfo.layout = pipelineLayout;                       // ���������� ���������
	pipelineInfo.renderPass = renderPass;                       // ��������� �������� � ��������������� �������� ����������
	pipelineInfo.subpass = 0;      // ��������� � ���-�������� (������ ���-������)
	pipelineInfo.stageCount = 1;




	// �������� ������������ ���������
	if (vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resultPipelineHandle) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating pipeline");
	}

	tools::LogMessage("Vulkan: Pipeline sucessfully initialized");

	// ��������� ������ ������ �� ����� ����� �������� ���������
	for (VkPipelineShaderStageCreateInfo &shaderStageInfo : shaderStages) {
		vkDestroyShaderModule(device.logicalDevice, shaderStageInfo.module, nullptr);
	}

	return resultPipelineHandle;
}