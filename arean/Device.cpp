
#include "mainRenderer.h"

VKStr::Device MainRenderer::initDevice(VkInstance vkInstance, VkSurfaceKHR surface, std::vector<const char*> extensionsRequired, 
	std::vector<const char*> validationLayersRequired, bool uniqueQueueFamilies)
{
	// Устройство которое будет возвращено
	VKStr::Device resDevice = {};

	// Получить кол-во видеокарт в системе
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

	// Если не нашлось видео-карт работающих с vulkan - ошибка
	if (deviceCount == 0) {
		throw std::runtime_error("Vulkan: Can't detect divice with Vulkan suppurt");
	}

	// Получить доступные видеокарты 
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physicalDevices.data());


	// Пройтись по всем видео-картам и проверить каждую на соответствие минимальным требованиям, устройство должно поддерживать
	for (const VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		// Получить информацию об очередях поддерживаемых устройством
		resDevice.queueFamilies = GetQueueFamilyInfo(physicalDevice, surface, uniqueQueueFamilies);

		// Если очереди данного устройства не совместимы с рендерингом - переходим к следующему
		if (!(resDevice.queueFamilies.IsRenderingCompatible())) {
			continue;
		}

		// Если данное устройство не поддерживает запрашиваемые расширения - переходим к следующему
		if (extensionsRequired.size() > 0 && !CheckDeviceExtensionSupported(physicalDevice, extensionsRequired)) {
			continue;
		}

		// Получить информацию о том как устройство может работать с поверхностью
		VKStr::SurfaceInfo surfaceInfo = GetSurfaceInfo(physicalDevice, surface);
		if (surfaceInfo.formats.empty() || surfaceInfo.presentModes.empty()) {
			continue;
		}

		// Записать хендл физического устройства, которое прошло все проверки
		resDevice.physicalDevice = physicalDevice;
	}

	// Если не нашлось видео-карт которые удовлетворяют всем требованиям - ошибка
	if (resDevice.physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Vulkan: Error in the 'InitDevice' function!");
	}

	// Массив объектов структуры VkDeviceQueueCreateInfo содержащих информацию для инициализации очередей
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	// Массив инедксов семейств (Графическое и представления)
	uint32_t queueFamilies[2] = { (uint32_t)resDevice.queueFamilies.graphics, (uint32_t)resDevice.queueFamilies.present };

	// Если графич. семейство и семейство представления - одно и то же (тот же индекс),
	// нет смысла создавать две очереди одного и того же семейства, можно обойтись одной
	for (int i = 0; i < (uniqueQueueFamilies ? 2 : 1); i++) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilies[i];
		queueCreateInfo.queueCount = 1;                                      // Выделяем одну очередь для каждого семейства
		queueCreateInfo.pQueuePriorities = nullptr;                          // Массив пр-тетов очередей в плане выделения ресурсов (одинаковые пр-теты, не используем)
		queueCreateInfos.push_back(queueCreateInfo);                         // Помещаем структуру в массив
	}


	// Информация о создаваемом логическом устройстве
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = (unsigned int)queueCreateInfos.size();

	// Проверка запрашиваемых расширений, указать если есть 
	if (!extensionsRequired.empty()) {
		if (!CheckDeviceExtensionSupported(resDevice.physicalDevice, extensionsRequired)) {
			throw std::runtime_error("Vulkan: Not all required device extensions supported.(device)");
		}

		deviceCreateInfo.enabledExtensionCount = (uint32_t)extensionsRequired.size();
		deviceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();
	}

	// Проверка запрашиваемых слоев валидации, указать если есть (если не доступны - ошибка)
	if (!validationLayersRequired.empty()) {
		if (!CheckValidationsLayersSupported(validationLayersRequired)) {
			throw std::runtime_error("Vulkan: Not all required validation layers supported. (device)");
		}

		deviceCreateInfo.enabledLayerCount = (uint32_t)validationLayersRequired.size();
		deviceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();
	}

	// Особенности устройства 
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// Создание логического устройства
	if (vkCreateDevice(resDevice.physicalDevice, &deviceCreateInfo, nullptr, &(resDevice.logicalDevice)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Failed to create logical device.");
	}

	// Получить хендлы очередей устройства (графической очереди и очереди представления)
	vkGetDeviceQueue(resDevice.logicalDevice, resDevice.queueFamilies.graphics, 0, &(resDevice.queues.graphics));
	vkGetDeviceQueue(resDevice.logicalDevice, resDevice.queueFamilies.present, 0, &(resDevice.queues.present));

	// Если в итоге устройство не готово - ошибка
	if (!resDevice.IsReady()) {
		throw std::runtime_error("Vulkan: Failed to initialize device and queues.");
	}

	// Сообщение об успешной инициализации устройства
	std::string deviceName = std::string(resDevice.GetProperties().deviceName);
	std::string message = "Vulkan: Device successfully initialized (" + deviceName + ")";
	tools::LogMessage(message);

	// Вернуть устройство
	return resDevice;
}

VKStr::QueueFamilyInfo MainRenderer::GetQueueFamilyInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool uniqueStrict)
{
	//Возвращаемый объект структуры QueueFamilyInfo с информацией о семействах
	VKStr::QueueFamilyInfo queueFamilyInfo;

	// Получить кол-во семейств очередей поддерживаемых физическим устройством 
	unsigned int queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	// Инициализировать массив семейств очередей и получить эти семейства
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	// Найти то семейство, которое поддерживает грфические команды, и записать его индекс в возвращаемый обьект
	for (unsigned int i = 0; i < queueFamilies.size(); i++) {
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFamilyInfo.graphics = i;
			break;
		}
	}

	// Найти то семейство, которое поддерживает представление и записать его индекс в возвращаемый объект
	for (unsigned int i = 0; i < queueFamilies.size(); i++) {

		// Если необходимо чтобы ID были уникальными 
		if (i == queueFamilyInfo.graphics && uniqueStrict) {
			continue;
		}

		// Поддержка представления 
		unsigned int presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

		if (queueFamilies[i].queueCount > 0 && presentSupport) {
			queueFamilyInfo.present = i;
			break;
		}
	}

	return queueFamilyInfo;
}

int MainRenderer::GetMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int typeFlags, VkMemoryPropertyFlags properties)
{
	// Получить настройки памяти физического устройства
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

	// Пройтись по всем типам и найти подходящий
	for (unsigned int i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeFlags & (1 << i)) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	// Отрицательное значение в случае отсутствия необходимого индекса
	return -1;
}

bool MainRenderer::CheckDeviceExtensionSupported(VkPhysicalDevice physicalDevice, std::vector<const char*> deviceExtensionsNames)
{
	// Доступные расширения устройства 
	std::vector<VkExtensionProperties> availableExtensions;

	// Получение кол-ва доступных расширений
	unsigned int deviceExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);

	// Если 0 то не поддерживается
	if (deviceExtensionCount == 0) {
		return false;
	}

	// Получение самих расширений
	availableExtensions.resize(deviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, availableExtensions.data());

	// Пройти по всем запрашиваемым расширениям, и проверить есть ли каждое в списке доступных
	for (const char* requiredExtName : deviceExtensionsNames) {
		bool found = false;
		for (const VkExtensionProperties &extProperties : availableExtensions) {
			if (strcmp(requiredExtName, extProperties.extensionName) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}

	return true;
}

void MainRenderer::DeinitDevice(VKStr::Device * device)
{
	device->Deinit();
	tools::LogMessage("Vulkan: Device successfully destroyed");
}