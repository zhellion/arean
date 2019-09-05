
#include "mainRenderer.h"

VkRenderPass MainRenderer::initRenderPassage(const VKStr::Device &device, VkSurfaceKHR surface, VkFormat colorAttachmentFormat, VkFormat depthStencilFormat)
{
	// Проверка доступности формата вложений (изображений)
	VKStr::SurfaceInfo surfaceInfo = GetSurfaceInfo(device.physicalDevice, surface);
	if (!surfaceInfo.IsFormatSupported(colorAttachmentFormat)) {
		throw std::runtime_error("Vulkan: Required surface format is not supported. (render-passage)");
	}

	// Проверка доступности формата глубины
	if (!device.IsDepthFormatSupported(depthStencilFormat)) {
		throw std::runtime_error("Vulkan: Required depth-stencil format is not supported. (render-passage)");
	}
	//this->shadowPass_.depthFormat = depthStencilFormat;

	// Массив описаний вложений
	std::vector<VkAttachmentDescription> attachments;

	// Описаниие цветового вложения (изображение)
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = colorAttachmentFormat;                                       // Формат цвета должен соответствовать тому что будет использован при создании своп-чейна
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;  //VK_SAMPLE_COUNT_1_BIT;                                      // Не использовать сглаживание !!
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                                 // На этапе начала прохода - очищать вложение
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;                               // На этапе конца прохода - хранить вложение (для дальнешей презентации)
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                      // Подресурс трафарета (начало прохода) - не используется
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                    // Подресурс трафарета (конце прохода) - не исрользуется
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                            // Размещение памяти в начале 
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;                        // Размещение памяти к которому вложение будет приведено после окончания прохода (для представления)
	attachments.push_back(colorAttachment);

	// Описание вложения глубины трафарета (z-буфер)
	VkAttachmentDescription depthStencilAttachment = {};
	depthStencilAttachment.format = depthStencilFormat;
	depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;								   // !!
	depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           // На этапе начала прохода - очищать вложение
	depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;                     // На этапе конца прохода - не имеет значение (память не используется для презентации, можно не хранить)
	depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                // Подресурс трафарета (начало прохода) - не используется
	depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // Подресурс трафарета (конце прохода) - не исрользуется
	depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // Размещение памяти в начале 
	depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Размещение памяти к которому вложение будет приведено после окончания прохода (глубина-трафарет)
	attachments.push_back(depthStencilAttachment);


	// Массив ссылок на цветовые вложения
	std::vector<VkAttachmentReference> colorAttachmentReferences = {
		{
			0,                                                       // Первый элемент массива вложений 
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL                 // Ожидается что размещение будет VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		}
	};

	// Ссылка на вложение глубины-трафарета
	VkAttachmentReference depthStencilAttachemntReference = {
		1,                                                           // Второй элемент массива вложений 
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL             // Ожидается что размещение будет VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	// Описание единственного под-прохода
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = (uint32_t)colorAttachmentReferences.size();  // Кол-во цветовых вложений
	subpassDescription.pColorAttachments = colorAttachmentReferences.data();               // Цветовые вложения (вложения для записи)
	subpassDescription.pDepthStencilAttachment = &depthStencilAttachemntReference;         // Глубина-трафарет (не используется)
	subpassDescription.inputAttachmentCount = 0;                                           // Кол-во входных вложений (не используются)
	subpassDescription.pInputAttachments = nullptr;                                        // Входные вложения (вложения для чтения, напр. того что было записано в пред-щем под-проходе)
	subpassDescription.preserveAttachmentCount = 0;                                        // Кол-во хранимых вложений (не используется)
	subpassDescription.pPreserveAttachments = nullptr;                                     // Хранимые вложения могут быть использованы для много-кратного использования в разных под-проходах
	subpassDescription.pResolveAttachments = nullptr;                                      // Resolve-вложения (полезны при работе со сглаживанием)!!!

	// Настройка зависимостей под-проходов

	std::vector<VkSubpassDependency> dependencies = {
		// Первая зависимость, начало конвейера
		// Перевод размещения от заключительного (final) к первоначальному (initial)
		{
			VK_SUBPASS_EXTERNAL,                                                       // Передыдущий под-проход - внешний, неявный
			0,                                                                         // Следующий под-проход - первый (и единсвтенный)
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // Подразумевается что предыдыщий под-проход (внешний) завершен на этапе завершения конвейера
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // Целевой подпроход (первый) начнется на этапе вывода цветовой информации из конвейера
			VK_ACCESS_MEMORY_READ_BIT,
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
		// Вторая зависимость, выход - конец конвейера
		// Перевод размещения от первоначального (initial) к заключительному (final)
		{
			0,                                                                         // Предыдщий под-проход - первый (и единсвтенный)
			VK_SUBPASS_EXTERNAL,                                                       // Следующий - внешний, неявный
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,                             // Предыдущий под-проход (первый) завершен на этапе вывода цветовой информации
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,                                      // Следующий под-проход (внешний) начат на этапе завершения конвейера
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_MEMORY_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
	};

	// Создать проход
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = (unsigned int)attachments.size();              //Кол-во описаний вложений
	renderPassInfo.pAttachments = attachments.data();                               //Описания вложений
	renderPassInfo.subpassCount = 1;                                                //Кол-вл под-проходов
	renderPassInfo.pSubpasses = &subpassDescription;                                //Описание под-проходов
	renderPassInfo.dependencyCount = (unsigned int)dependencies.size();             //Кол-во зависимсотей
	renderPassInfo.pDependencies = dependencies.data();                             //Зависимости

	//Создание прохода
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

	// Массив описаний вложений
	std::vector<VkAttachmentDescription> attachments;


	// Описание вложения глубины трафарета (z-буфер)
	VkAttachmentDescription depthStencilAttachment = {};
	depthStencilAttachment.format = shadowPass_.depthFormat;
	depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;								   
	depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;                           
	depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;                // Подресурс трафарета (начало прохода) - не используется
	depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;              // Подресурс трафарета (конце прохода) - не исрользуется
	depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;                      // Размещение памяти в начале 
	depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL; // Размещение памяти к которому вложение будет приведено после окончания прохода (глубина-трафарет)
	attachments.push_back(depthStencilAttachment);


	// Ссылка на вложение глубины-трафарета
	VkAttachmentReference depthStencilAttachemntReference = {
		0,                                                           
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
	};

	// Описание единственного под-прохода
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 0;  // Кол-во цветовых вложений
	subpassDescription.pDepthStencilAttachment = &depthStencilAttachemntReference;         


	// Настройка зависимостей под-проходов

	std::vector<VkSubpassDependency> dependencies = {
		// Первая зависимость, начало конвейера
		// Перевод размещения от заключительного (final) к первоначальному (initial)
		{
			VK_SUBPASS_EXTERNAL,                                                       // Передыдущий под-проход - внешний, неявный
			0,                                                                         // Следующий под-проход - первый (и единсвтенный)
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,                                      // Подразумевается что предыдыщий под-проход (внешний) завершен на этапе завершения конвейера
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,                             // Целевой подпроход (первый) начнется на этапе вывода цветовой информации из конвейера
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		},
		// Вторая зависимость, выход - конец конвейера
		// Перевод размещения от первоначального (initial) к заключительному (final)
		{
			0,                                                                         // Предыдщий под-проход - первый (и единсвтенный)
			VK_SUBPASS_EXTERNAL,                                                       // Следующий - внешний, неявный
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,                             // Предыдущий под-проход (первый) завершен на этапе вывода цветовой информации
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,                                      // Следующий под-проход (внешний) начат на этапе завершения конвейера
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT
		}
	};

	// Создать проход
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = attachments.size();              //Кол-во описаний вложений
	renderPassInfo.pAttachments = attachments.data();                               //Описания вложений
	renderPassInfo.subpassCount = 1;                                                //Кол-вл под-проходов
	renderPassInfo.pSubpasses = &subpassDescription;                                //Описание под-проходов
	renderPassInfo.dependencyCount = (unsigned int)dependencies.size();             //Кол-во зависимсотей
	renderPassInfo.pDependencies = dependencies.data();                             //Зависимости

	//Создание прохода
	VkRenderPass renderPass;
	if (vkCreateRenderPass(device.logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Failed to create render passage");
	}

	tools::LogMessage("Vulkan: Render passage successfully initialized");

	return renderPass;
}