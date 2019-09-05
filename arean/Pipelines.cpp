
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

	// Конфигурация привязок и аттрибутов входных данных (вершинных)
	std::vector<VkVertexInputBindingDescription> bindingDescription = GetVertexInputBindingDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = GetVertexInputAttributeDescriptions(0);

	// Конфигурация стадии ввода вершинных данных
	VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
	vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStage.vertexBindingDescriptionCount = (uint32_t)bindingDescription.size();
	vertexInputStage.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputStage.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Описание этапа "сборки" входных данных
	// Конвейер будет "собирать" вершинны в набор треугольников
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage = {};
	inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;					// Список треугольников
	inputAssemblyStage.primitiveRestartEnable = VK_FALSE;								// Перезагрузка примитивов не используется

	// Прогамируемые (шейдерные) этапы конвейера
	// Используем 2 шейдера - вершинный (для каждой вершины) и фрагментный 
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

	// Описываем область просмотра
	// Размеры равны размерам изображений swap-chain, которые в свою очередь равны размерам поверхности отрисовки
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain.imageExtent.width;
	viewport.height = (float)swapchain.imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Настройки обрезки изображения (не обрезать, размеры совпадают с областью просмотра)
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.imageExtent;

	// Описываем этап вывода в область просмотра
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Описываем этап растеризации
	VkPipelineRasterizationStateCreateInfo rasterizationStage = {};
	rasterizationStage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStage.depthClampEnable = VK_FALSE;                     // Фрагменты за ближней и дальней гранью камеры отбрасываются (VK_TRUE для противоположного эффекта)
	rasterizationStage.rasterizerDiscardEnable = VK_FALSE;              // Отключение растеризации геометрии - не нужно (VK_TRUE для противоположного эффекта)
	rasterizationStage.polygonMode = VK_POLYGON_MODE_FILL;              // Закрашенные полигоны, для точек и линий тоже пойдет//https://vulkan.lunarg.com/doc/view/1.0.37.0/linux/vkspec.chunked/ch24s07.html#VkPolygonMode
	rasterizationStage.lineWidth = 1.0f;                                // Ширина линии
	rasterizationStage.cullMode = VK_CULL_MODE_BACK_BIT;                // Отсечение граней (отсекаются те, что считаются задними)!!!
	rasterizationStage.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//VK_FRONT_FACE_CLOCKWISE;             // Порялок следования вершин для лицевой грани - по часовой стрелке!!!
	rasterizationStage.depthBiasEnable = VK_FALSE;                      // Контроль значений глубины
	rasterizationStage.depthBiasConstantFactor = 0.0f;
	rasterizationStage.depthBiasClamp = 0.0f;
	rasterizationStage.depthBiasSlopeFactor = 0.0f;

	// Описываем этап z-теста (теста глубины)
	// Активировать тест глубины, использовать сравнение "меньше или равно"
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

	// Описываем этап мультисемплинга (сглаживание пиксельных лесенок)
	// Мултисемплинг - добавляет дополнительные ключевые точки в пиксел (семплы), для более точного вычисления его цвета
	VkPipelineMultisampleStateCreateInfo multisamplingStage = {};
	multisamplingStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingStage.sampleShadingEnable = VK_TRUE;
	multisamplingStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  //VK_SAMPLE_COUNT_1_BIT;//сглаживание 1 пиксель
	multisamplingStage.minSampleShading = 1.0f;
	multisamplingStage.pSampleMask = nullptr;
	multisamplingStage.alphaToCoverageEnable = VK_FALSE;
	multisamplingStage.alphaToOneEnable = VK_FALSE;

	// Этап смешивания цвета (для каждого фрейм-буфера)
	// На этом этапе можно настроить как должны отображаться, например, полупрозрачные объекты
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// Глобальные настройки смешивания
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachment;

	// Информация инициализации графического конвейера
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)shaderStages.size();    // Кол-во шейдерных этапов
	pipelineInfo.pStages = shaderStages.data();                 // Шейдерные этапы (их конфигураци)
	pipelineInfo.pVertexInputState = &vertexInputStage;         // Настройки этапа ввода вершинных данных
	pipelineInfo.pInputAssemblyState = &inputAssemblyStage;     // Настройки этапа сборки примитивов из полученных вершин
	pipelineInfo.pViewportState = &viewportState;               // Настройки области видимости
	pipelineInfo.pRasterizationState = &rasterizationStage;     // Настройки этапа растеризации
	pipelineInfo.pDepthStencilState = &depthStencilStage;       // Настройка этапа z-теста
	pipelineInfo.pMultisampleState = &multisamplingStage;       // Настройки этапа мультисемплинга
	pipelineInfo.pColorBlendState = &colorBlendState;           // Настройки этапа смешивания цветов
	pipelineInfo.layout = pipelineLayout;                       // Размещение конвейера
	pipelineInfo.renderPass = renderPass;                       // Связываем конвейер с соответствующим проходом рендеринга
	pipelineInfo.subpass = 0;                                   // Связываем с под-проходом (первый под-проход)

	// Создание графического конвейера
	if (vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resultPipelineHandle) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating pipeline");
	}

	tools::LogMessage("Vulkan: Pipeline sucessfully initialized");

	// Шейдерные модули больше не нужны после создания конвейера
	for (VkPipelineShaderStageCreateInfo &shaderStageInfo : shaderStages) {
		vkDestroyShaderModule(device.logicalDevice, shaderStageInfo.module, nullptr);
	}

	return resultPipelineHandle;
}
//копия создания очереди, только с обратное трассировкой треугольников, использовано для скайбокса.
VkPipeline MainRenderer::initSBGraphicsPipeline(const VKStr::Device & device, VkPipelineLayout pipelineLayout, const VKStr::Swapchain & swapchain, VkRenderPass renderPass, std::string fShaderName, std::string vShaderName)
{
	VkPipeline resultPipelineHandle = VK_NULL_HANDLE;

	// Конфигурация привязок и аттрибутов входных данных (вершинных)
	std::vector<VkVertexInputBindingDescription> bindingDescription = GetVertexInputBindingDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = GetVertexInputAttributeDescriptions(0);

	// Конфигурация стадии ввода вершинных данных
	VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
	vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStage.vertexBindingDescriptionCount = (uint32_t)bindingDescription.size();
	vertexInputStage.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputStage.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Описание этапа "сборки" входных данных
	// Конвейер будет "собирать" вершинны в набор треугольников
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage = {};
	inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;					// Список треугольников
	inputAssemblyStage.primitiveRestartEnable = VK_FALSE;								// Перезагрузка примитивов не используется

	// Прогамируемые (шейдерные) этапы конвейера
	// Используем 2 шейдера - вершинный (для каждой вершины) и фрагментный 
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

	// Описываем область просмотра
	// Размеры равны размерам изображений swap-chain, которые в свою очередь равны размерам поверхности отрисовки
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapchain.imageExtent.width;
	viewport.height = (float)swapchain.imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Настройки обрезки изображения (не обрезать, размеры совпадают с областью просмотра)
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapchain.imageExtent;

	// Описываем этап вывода в область просмотра
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Описываем этап растеризации
	VkPipelineRasterizationStateCreateInfo rasterizationStage = {};
	rasterizationStage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStage.depthClampEnable = VK_FALSE;                     // Фрагменты за ближней и дальней гранью камеры отбрасываются (VK_TRUE для противоположного эффекта)
	rasterizationStage.rasterizerDiscardEnable = VK_FALSE;              // Отключение растеризации геометрии - не нужно (VK_TRUE для противоположного эффекта)
	rasterizationStage.polygonMode = VK_POLYGON_MODE_FILL;              // Закрашенные полигоны, для точек и линий тоже пойдет//https://vulkan.lunarg.com/doc/view/1.0.37.0/linux/vkspec.chunked/ch24s07.html#VkPolygonMode
	rasterizationStage.lineWidth = 1.0f;                                // Ширина линии
	rasterizationStage.cullMode = VK_CULL_MODE_FRONT_BIT;                // Отсечение граней (отсекаются те, что считаются передними)!!!
	rasterizationStage.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//VK_FRONT_FACE_CLOCKWISE;             // Порялок следования вершин для лицевой грани - по часовой стрелке!!!
	rasterizationStage.depthBiasEnable = VK_FALSE;                      // Контроль значений глубины
	rasterizationStage.depthBiasConstantFactor = 0.0f;
	rasterizationStage.depthBiasClamp = 0.0f;
	rasterizationStage.depthBiasSlopeFactor = 0.0f;

	// Описываем этап z-теста (теста глубины)
	// Активировать тест глубины, использовать сравнение "меньше или равно"
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

	// Описываем этап мультисемплинга (сглаживание пиксельных лесенок)
	// Мултисемплинг - добавляет дополнительные ключевые точки в пиксел (семплы), для более точного вычисления его цвета
	VkPipelineMultisampleStateCreateInfo multisamplingStage = {};
	multisamplingStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingStage.sampleShadingEnable = VK_TRUE;
	multisamplingStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  //VK_SAMPLE_COUNT_1_BIT;//сглаживание 1 пиксель
	multisamplingStage.minSampleShading = 1.0f;
	multisamplingStage.pSampleMask = nullptr;
	multisamplingStage.alphaToCoverageEnable = VK_FALSE;
	multisamplingStage.alphaToOneEnable = VK_FALSE;

	// Этап смешивания цвета (для каждого фрейм-буфера)
	// На этом этапе можно настроить как должны отображаться, например, полупрозрачные объекты
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// Глобальные настройки смешивания
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachment;

	// Информация инициализации графического конвейера
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)shaderStages.size();    // Кол-во шейдерных этапов
	pipelineInfo.pStages = shaderStages.data();                 // Шейдерные этапы (их конфигураци)
	pipelineInfo.pVertexInputState = &vertexInputStage;         // Настройки этапа ввода вершинных данных
	pipelineInfo.pInputAssemblyState = &inputAssemblyStage;     // Настройки этапа сборки примитивов из полученных вершин
	pipelineInfo.pViewportState = &viewportState;               // Настройки области видимости
	pipelineInfo.pRasterizationState = &rasterizationStage;     // Настройки этапа растеризации
	pipelineInfo.pDepthStencilState = &depthStencilStage;       // Настройка этапа z-теста
	pipelineInfo.pMultisampleState = &multisamplingStage;       // Настройки этапа мультисемплинга
	pipelineInfo.pColorBlendState = &colorBlendState;           // Настройки этапа смешивания цветов
	pipelineInfo.layout = pipelineLayout;                       // Размещение конвейера
	pipelineInfo.renderPass = renderPass;                       // Связываем конвейер с соответствующим проходом рендеринга
	pipelineInfo.subpass = 0;                                   // Связываем с под-проходом (первый под-проход)

	// Создание графического конвейера
	if (vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resultPipelineHandle) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating pipeline");
	}

	tools::LogMessage("Vulkan: Pipeline sucessfully initialized");

	// Шейдерные модули больше не нужны после создания конвейера
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
			bindingIndex,                   // Индекс привязки вершинных буферов
			sizeof(VKStr::Vertex),      // Размерность шага
			VK_VERTEX_INPUT_RATE_VERTEX     // Правила перехода к следующим
		}
	};
}

std::vector<VkVertexInputAttributeDescription> MainRenderer::GetVertexInputAttributeDescriptions(unsigned int bindingIndex)
{
	return
	{
		{
			0,                                      // Индекс аттрибута (location в шейдере)
			bindingIndex,                           // Индекс привязки вершинных буферов
			VK_FORMAT_R32G32B32_SFLOAT,             // Тип аттрибута (соответствует vec3 у шейдера)
			offsetof(VKStr::Vertex, position)       // Cдвиг в структуре
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
			VK_FORMAT_R32G32_SFLOAT,               // Тип аттрибута (соответствует vec2 у шейдера)
			offsetof(VKStr::Vertex, texCoord)
		},
		{
			3,
			bindingIndex,
			VK_FORMAT_R32_UINT,                    // Тип аттрибута (соответствует uint у шейдера)
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
	// Размер
	size_t shaderSize;

	// Содержимое файла (код шейдера)
	char* shaderCode = nullptr;

	// Загрузить код шейдера
	bool loaded = tools::LoadBytesFromFile(tools::DirProg() + "/../shaders/" + filename, &shaderCode, &shaderSize);

	// Если не удалось загрузить или файл пуст
	if (!loaded || shaderSize == 0) {
		std::string msg = "Vulkan: Error while loading shader code from file " + filename;
		throw std::runtime_error(msg);
	}

	// Конфигурация шейдерного модуля
	VkShaderModuleCreateInfo moduleCreateInfo{};
	moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleCreateInfo.codeSize = shaderSize;
	moduleCreateInfo.pCode = (unsigned int*)shaderCode;

	// Создать шейдерный модуль
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice, &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		std::string msg = "Vulkan: Error whiler creating shader module from file " + filename;
		throw std::runtime_error(msg);
	}

	// Удаляем код шейдера (хранить его более не нужно, поскольку он был передан в шейдерный модуль)
	delete[] shaderCode;

	
	return shaderModule;
}
//--------------------------------------------------------------------------------
//shadowmap
VkPipeline MainRenderer::initShadowPipeline(const VKStr::Device & device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, std::string vShaderName)
{
	VkPipeline resultPipelineHandle = VK_NULL_HANDLE;


	// Конфигурация привязок и аттрибутов входных данных (вершинных)
	std::vector<VkVertexInputBindingDescription> bindingDescription = GetVertexInputBindingDescriptions(0);
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions = GetVertexInputAttributeDescriptions(0);

	// Конфигурация стадии ввода вершинных данных
	VkPipelineVertexInputStateCreateInfo vertexInputStage = {};
	vertexInputStage.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStage.vertexBindingDescriptionCount = (uint32_t)bindingDescription.size();
	vertexInputStage.pVertexBindingDescriptions = bindingDescription.data();
	vertexInputStage.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
	vertexInputStage.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Описание этапа "сборки" входных данных
	// Конвейер будет "собирать" вершинны в набор треугольников
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStage = {};
	inputAssemblyStage.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStage.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;					// Список треугольников
	inputAssemblyStage.primitiveRestartEnable = VK_FALSE;								// Перезагрузка примитивов не используется

	// Прогамируемые (шейдерные) этапы конвейера
	// используем 1 щейдер (вершинный)
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

	// Описываем область просмотра
	// Размеры равны размерам изображений swap-chain, которые в свою очередь равны размерам поверхности отрисовки
	VkViewport viewport = {};
	viewport.width = (float)shadowPass_.width;
	viewport.height = (float)shadowPass_.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Настройки обрезки изображения (не обрезать, размеры совпадают с областью просмотра)
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent.width = shadowPass_.width;
	scissor.extent.height = shadowPass_.height;

	// Описываем этап вывода в область просмотра
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	viewportState.flags = 0;

	// Описываем этап растеризации
	VkPipelineRasterizationStateCreateInfo rasterizationStage = {};
	rasterizationStage.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStage.depthClampEnable = VK_FALSE;                     // Фрагменты за ближней и дальней гранью камеры отбрасываются (VK_TRUE для противоположного эффекта)
	//rasterizationStage.rasterizerDiscardEnable = VK_FALSE;              // Отключение растеризации геометрии - не нужно (VK_TRUE для противоположного эффекта)
	rasterizationStage.polygonMode = VK_POLYGON_MODE_FILL;              // Закрашенные полигоны, для точек и линий тоже пойдет//https://vulkan.lunarg.com/doc/view/1.0.37.0/linux/vkspec.chunked/ch24s07.html#VkPolygonMode
	rasterizationStage.lineWidth = 1.0f;                                // Ширина линии
	rasterizationStage.cullMode = VK_CULL_MODE_BACK_BIT;                // Отсечение граней (отсекаются те, что считаются задними)!!!
	rasterizationStage.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//VK_FRONT_FACE_CLOCKWISE;             // Порялок следования вершин для лицевой грани - по часовой стрелке!!!
	rasterizationStage.depthBiasEnable = VK_TRUE;                      // Контроль значений глубины
	rasterizationStage.depthBiasConstantFactor = 1.25f;
	rasterizationStage.depthBiasClamp = 0.0f;
	rasterizationStage.depthBiasSlopeFactor = 1.75f;

	// Описываем этап z-теста (теста глубины)
	// Активировать тест глубины, использовать сравнение "меньше или равно"
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

	// Описываем этап мультисемплинга (сглаживание пиксельных лесенок)
	// Мултисемплинг - добавляет дополнительные ключевые точки в пиксел (семплы), для более точного вычисления его цвета
	VkPipelineMultisampleStateCreateInfo multisamplingStage = {};
	multisamplingStage.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	//multisamplingStage.sampleShadingEnable = VK_TRUE;
	multisamplingStage.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  //VK_SAMPLE_COUNT_1_BIT;//сглаживание 1 пиксель
	//multisamplingStage.minSampleShading = 1.0f;
	//multisamplingStage.pSampleMask = nullptr;
	//multisamplingStage.alphaToCoverageEnable = VK_FALSE;
	//multisamplingStage.alphaToOneEnable = VK_FALSE;
	multisamplingStage.flags = 0;

	// Этап смешивания цвета (для каждого фрейм-буфера)
	// На этом этапе можно настроить как должны отображаться, например, полупрозрачные объекты
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = 0xf;
	colorBlendAttachment.blendEnable = VK_FALSE;
	//colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	//colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	//colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	//colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	//colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	//colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// Глобальные настройки смешивания
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

	// Информация инициализации графического конвейера
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = (uint32_t)shaderStages.size();    // Кол-во шейдерных этапов
	pipelineInfo.pStages = shaderStages.data();                 // Шейдерные этапы (их конфигураци)
	pipelineInfo.pVertexInputState = &vertexInputStage;         // Настройки этапа ввода вершинных данных
	pipelineInfo.pInputAssemblyState = &inputAssemblyStage;     // Настройки этапа сборки примитивов из полученных вершин
	pipelineInfo.pViewportState = &viewportState;               // Настройки области видимости
	pipelineInfo.pRasterizationState = &rasterizationStage;     // Настройки этапа растеризации
	pipelineInfo.pDepthStencilState = &depthStencilStage;       // Настройка этапа z-теста
	pipelineInfo.pMultisampleState = &multisamplingStage;       // Настройки этапа мультисемплинга
	pipelineInfo.pColorBlendState = &colorBlendState;           // Настройки этапа смешивания цветов
	pipelineInfo.pDynamicState = &dynamicStateCI;
	pipelineInfo.layout = pipelineLayout;                       // Размещение конвейера
	pipelineInfo.renderPass = renderPass;                       // Связываем конвейер с соответствующим проходом рендеринга
	pipelineInfo.subpass = 0;      // Связываем с под-проходом (первый под-проход)
	pipelineInfo.stageCount = 1;




	// Создание графического конвейера
	if (vkCreateGraphicsPipelines(device.logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &resultPipelineHandle) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating pipeline");
	}

	tools::LogMessage("Vulkan: Pipeline sucessfully initialized");

	// Шейдерные модули больше не нужны после создания конвейера
	for (VkPipelineShaderStageCreateInfo &shaderStageInfo : shaderStages) {
		vkDestroyShaderModule(device.logicalDevice, shaderStageInfo.module, nullptr);
	}

	return resultPipelineHandle;
}