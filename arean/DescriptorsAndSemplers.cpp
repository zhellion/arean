
#include "mainRenderer.h"


//----------------------------------------------------------------
//Общие функции
void MainRenderer::DeinitDescriptorSet(const VKStr::Device & device, VkDescriptorPool descriptorPool, VkDescriptorSet * descriptorSet)
{
	if (device.logicalDevice != VK_NULL_HANDLE && descriptorPool != VK_NULL_HANDLE && descriptorSet != nullptr && *descriptorSet != VK_NULL_HANDLE)
	{
		if (vkFreeDescriptorSets(device.logicalDevice, descriptorPool, 1, descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("Vulkan: Error while destroying descriptor set");
		}
		*descriptorSet = VK_NULL_HANDLE;

		tools::LogMessage("Vulkan: Descriptor set successfully deinitialized");
	}
}

void MainRenderer::DeinitDescriporSetLayout(const VKStr::Device & device, VkDescriptorSetLayout * descriptorSetLayout)
{
	if (device.logicalDevice != VK_NULL_HANDLE && descriptorSetLayout != nullptr && *descriptorSetLayout != VK_NULL_HANDLE) {
		vkDestroyDescriptorSetLayout(device.logicalDevice, *descriptorSetLayout, nullptr);
		*descriptorSetLayout = VK_NULL_HANDLE;

		tools::LogMessage("Vulkan: Descriptor set layout successfully deinitialized");
	}
}

void MainRenderer::DeinitDescriptorPool(const VKStr::Device & device, VkDescriptorPool * descriptorPool)
{
	if (descriptorPool != nullptr && *descriptorPool != VK_NULL_HANDLE) {
		vkDestroyDescriptorPool(device.logicalDevice, *descriptorPool, nullptr);
		*descriptorPool = VK_NULL_HANDLE;
		tools::LogMessage("Vulkan: Descriptor pool successfully deinitialized");
	}
}

//----------------------------------------------
//сэмплер
VkSampler MainRenderer::initTextureSampler(const VKStr::Device & device)
{

	VkSampler resultSampler;

	// Настройка семплера
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;                      // Тип интерполяции когда тексели больше фрагментов
	samplerInfo.minFilter = VK_FILTER_LINEAR;                      // Тип интерполяции когда тексели меньше фрагментов
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;     // Повторять при выходе за пределы
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;                        // Включть анизотропную фильтрацию
	samplerInfo.maxAnisotropy = 16;                                 // уровень фильтрации
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;    // Цвет грани
	samplerInfo.unnormalizedCoordinates = VK_FALSE;                // Использовать нормальзованные координаты (не пиксельные)
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	// Создание семплера
	if (vkCreateSampler(device.logicalDevice, &samplerInfo, nullptr, &resultSampler) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating texture sampler");
	}

	tools::LogMessage("Vulkan: Texture sampler successfully initialized");

	return resultSampler;
}

void MainRenderer::DeinitTextureSampler(const VKStr::Device & device, VkSampler * sampler)
{
	if (sampler != nullptr && *sampler != VK_NULL_HANDLE) {
		vkDestroySampler(device.logicalDevice, *sampler, nullptr);
		*sampler = VK_NULL_HANDLE;
	}
}

VkDescriptorPool MainRenderer::initDescriptorPoolTextures(const VKStr::Device & device, uint32_t maxDescriptorSets)
{

	VkDescriptorPool descriptorPoolResult = VK_NULL_HANDLE;

	// Парамтеры размеров пула
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// Один дескриптор для текстурного семплера
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1 },
	};

	// Конфигурация пула
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = maxDescriptorSets;

	// Создание дескрипторного пула
	if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool (texture)");
	}

	tools::LogMessage("Vulkan: Texture descriptor pool (texture) successfully initialized");


	return descriptorPoolResult;
}

VkDescriptorSetLayout MainRenderer::initDescriptorSetLayoutTextures(const VKStr::Device & device)
{

	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;


	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,    // Тип дескриптора (семплер изображения)
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_FRAGMENT_BIT,                 // Этап конвейера (фрагметный шейдер)
			nullptr
		}

	};



	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.bindingCount = (uint32_t)bindings.size();
	descriptorLayoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayoutInfo, nullptr, &layoutResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout (texture)");
	}

	tools::LogMessage("Vulkan: Texture descriptor set layout successfully initialized (texture)");

	return layoutResult;
}

VkDescriptorSet MainRenderer::initShadowSamplerDescriptorSet(const VKStr::Device & device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
	VKStr::ShadowPass shadowPass)
{
	VkDescriptorSet descriptorSetResult = VK_NULL_HANDLE;

	// Получить новый набор дескрипторов из дескриптороного пула
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocInfo, &descriptorSetResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
	}

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;//VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	imageInfo.imageView = shadowPass.depth.view;
	imageInfo.sampler = shadowPass.depthSampler;


	// Конфигурация добавляемых в набор дескрипторов
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
			nullptr,                                     // pNext
			descriptorSetResult,						// Целевой набор дескрипторов
			0,                                           // Точка привязки (у шейдера)
			0,                                           // Элемент массив (массив не используется)
			1,                                           // Кол-во дескрипторов
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,   // Тип дескриптора
			&imageInfo,                                  // Информация о параметрах изображения
			nullptr,
			nullptr
		}
	};

	// Обновить наборы дескрипторов
	vkUpdateDescriptorSets(this->device_.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);


	tools::LogMessage("Vulkan: Descriptor set successfully initialized");

	// Вернуть хендл набора
	return descriptorSetResult;
}

//-------------------------------------------------------------------------
//Дескриптор главного сета униформ
VkDescriptorPool MainRenderer::initDescriptorPoolMain(const VKStr::Device &device)
{

	VkDescriptorPool descriptorPoolResult = VK_NULL_HANDLE;

	// Парамтеры размеров пула
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// Один дескриптор для глобального uniform-буфера
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , 1 },
		// Один дескриптор для unform-буферов отдельных объектов (динамический)
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }
	};


	// Конфигурация пула
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = 1;

	// Создание дескрипторного пула
	if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool");
	}

	tools::LogMessage("Vulkan: Main descriptor pool successfully initialized");


	return descriptorPoolResult;
}

VkDescriptorSetLayout MainRenderer::initDescriptorSetLayoutMain(const VKStr::Device & device)
{

	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;

	// привязки дескрипторов к этапам конвейера

	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // Тип дескриптора (буфер формы, обычный)
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_VERTEX_BIT,                   // Этап конвейера (вершинный шейдер)
			nullptr
		},
		{
			1,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    // Тип дескриптора (буфер формы, динамический)
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_VERTEX_BIT| VK_SHADER_STAGE_FRAGMENT_BIT,                   // Этап конвейера (вершинный шейдер)
			nullptr
		}
	};

	// Инициализировать размещение дескрипторного набора
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.bindingCount = (uint32_t)bindings.size();
	descriptorLayoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayoutInfo, nullptr, &layoutResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
	}

	tools::LogMessage("Vulkan: Main descriptor set layout successfully initialized");

	return layoutResult;
}

VkDescriptorSetLayout MainRenderer::initShadowDescriptorSetLayoutMain(const VKStr::Device & device)
{

	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;

	// привязки дескрипторов к этапам конвейера

	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // Тип дескриптора (буфер формы, обычный)
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_VERTEX_BIT,                   // Этап конвейера (вершинный шейдер)
			nullptr
		},
		{
			1,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    // Тип дескриптора (буфер формы, динамический)
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_VERTEX_BIT,                   // Этап конвейера (вершинный шейдер)
			nullptr
		}
	};

	// Инициализировать размещение дескрипторного набора
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.bindingCount = (uint32_t)bindings.size();
	descriptorLayoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayoutInfo, nullptr, &layoutResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
	}

	tools::LogMessage("Vulkan: Main descriptor set layout successfully initialized");

	return layoutResult;
}

VkDescriptorSet MainRenderer::initDescriptorSetMain(const VKStr::Device & device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
	const VKStr::UniformBuffer & uniformBufferWorld, const VKStr::UniformBuffer & uniformBufferModels)
{
	VkDescriptorSet descriptorSetResult = VK_NULL_HANDLE;

	// Получить новый набор дескрипторов из дескриптороного пула
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocInfo, &descriptorSetResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
	}

	// Конфигурация добавляемых в набор дескрипторов
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
			nullptr,                                     // pNext
			descriptorSetResult,                         // Целевой набор дескрипторов
			0,                                           // Точка привязки (у шейдера)
			0,                                           // Элемент массив (массив не используется)
			1,                                           // Кол-во дескрипторов
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,           // Тип дескриптора
			nullptr,
			&(uniformBufferWorld.descriptorBufferInfo),  // Информация о параметрах буфера
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
			nullptr,                                     // pNext
			descriptorSetResult,                         // Целевой набор дескрипторов
			1,                                           // Точка привязки (у шейдера)
			0,                                           // Элемент массив (массив не используется)
			1,                                           // Кол-во дескрипторов
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,   // Тип дескриптора
			nullptr,
			&(uniformBufferModels.descriptorBufferInfo), // Информация о параметрах буфера
			nullptr,
		},
	};

	// Обновить наборы дескрипторов
	vkUpdateDescriptorSets(device.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

	tools::LogMessage("Vulkan: Descriptor set successfully initialized");

	// Вернуть хендл набора
	return descriptorSetResult;
}

//------------------------------------------------------------------
//Дескрипторы сета униформ источников света

VkDescriptorPool MainRenderer::initDescriptorPoolLight(const VKStr::Device &device)
{

	VkDescriptorPool descriptorPoolResult = VK_NULL_HANDLE;

	// Парамтеры размеров пула
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// Один дескриптор для глобального uniform-буфера
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		
	};


	// Конфигурация пула
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = 1;

	// Создание дескрипторного пула
	if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool (Light)");
	}

	tools::LogMessage("Vulkan: Main descriptor pool successfully initialized");


	return descriptorPoolResult;
}

VkDescriptorSetLayout MainRenderer::initDescriptorSetLayoutLight(const VKStr::Device & device)
{

	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;

	// привязки дескрипторов к этапам конвейера

	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_FRAGMENT_BIT,                   // Этап конвейера (вершинный шейдер)
			nullptr
		}
	};

	// Инициализировать размещение дескрипторного набора
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.bindingCount = (uint32_t)bindings.size();
	descriptorLayoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayoutInfo, nullptr, &layoutResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
	}

	tools::LogMessage("Vulkan: Main descriptor set layout successfully initialized");

	return layoutResult;
}

VkDescriptorSet MainRenderer::initDescriptorSetLight(const VKStr::Device & device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
	const VKStr::UniformBuffer & uniformBufferLCount)
{
	VkDescriptorSet descriptorSetResult = VK_NULL_HANDLE;

	// Получить новый набор дескрипторов из дескриптороного пула
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocInfo, &descriptorSetResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
	}

	// Конфигурация добавляемых в набор дескрипторов
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
			nullptr,                                     // pNext
			descriptorSetResult,                         // Целевой набор дескрипторов
			0,                                           // Точка привязки (у шейдера)
			0,                                           // Элемент массив (массив не используется)
			1,                                           // Кол-во дескрипторов
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,           // Тип дескриптора
			nullptr,
			&(uniformBufferLCount.descriptorBufferInfo),  // Информация о параметрах буфера
			nullptr
		}
	};

	// Обновить наборы дескрипторов
	vkUpdateDescriptorSets(device.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

	tools::LogMessage("Vulkan: Descriptor set successfully initialized");

	// Вернуть хендл набора
	return descriptorSetResult;
}
//--------------------------------------------------------
//вектора света
VkDescriptorPool MainRenderer::initDescriptorPoolVLight(const VKStr::Device &device)
{

	VkDescriptorPool descriptorPoolResult = VK_NULL_HANDLE;

	// Парамтеры размеров пула
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// Один дескриптор для глобального uniform-буфера
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },

	};


	// Конфигурация пула
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = 1;

	// Создание дескрипторного пула
	if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool (Light)");
	}

	tools::LogMessage("Vulkan: Main descriptor pool successfully initialized");


	return descriptorPoolResult;
}

VkDescriptorSetLayout MainRenderer::initDescriptorSetLayoutVLight(const VKStr::Device & device)
{

	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;

	// привязки дескрипторов к этапам конвейера

	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // Индекс привязки
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,                                            // Кол-во дескрипторов
			VK_SHADER_STAGE_VERTEX_BIT |VK_SHADER_STAGE_FRAGMENT_BIT,                   // Этап конвейера (вершинный шейдер)
			nullptr
		}
	};

	// Инициализировать размещение дескрипторного набора
	VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
	descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorLayoutInfo.pNext = nullptr;
	descriptorLayoutInfo.bindingCount = (uint32_t)bindings.size();
	descriptorLayoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device.logicalDevice, &descriptorLayoutInfo, nullptr, &layoutResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorSetLayout. Can't initialize descriptor set layout");
	}

	tools::LogMessage("Vulkan: Main descriptor set layout successfully initialized");

	return layoutResult;
}

VkDescriptorSet MainRenderer::initDescriptorSetVLight(const VKStr::Device & device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
	const VKStr::UniformBuffer & uniformBufferVectorsLight)
{
	VkDescriptorSet descriptorSetResult = VK_NULL_HANDLE;

	// Получить новый набор дескрипторов из дескриптороного пула
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocInfo, &descriptorSetResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
	}

	// Конфигурация добавляемых в набор дескрипторов
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
			nullptr,                                     // pNext
			descriptorSetResult,                         // Целевой набор дескрипторов
			0,                                           // Точка привязки (у шейдера)
			0,                                           // Элемент массив (массив не используется)
			1,                                           // Кол-во дескрипторов
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			// Тип дескриптора
			nullptr,
			&(uniformBufferVectorsLight.descriptorBufferInfo), // Информация о параметрах буфера
			nullptr,
		}
	};

	// Обновить наборы дескрипторов
	vkUpdateDescriptorSets(device.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

	tools::LogMessage("Vulkan: Descriptor set successfully initialized");

	// Вернуть хендл набора
	return descriptorSetResult;

}
