
#include "mainRenderer.h"

VkInstance MainRenderer::initInstance(
	std::string applicationName,
	std::string engineName,
	std::vector<const char*> extensionsRequired,
	std::vector<const char*> validationLayersRequired)
{
	// Структура с информацией о приложении
	VkApplicationInfo applicationInfo = {};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = applicationName.c_str();
	applicationInfo.pEngineName = engineName.c_str();
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	// Структура с информацией экземпляре vulkan

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;

	// Запрашивается ли валидация
	bool validationQueried = false;

	// Расширения
	if (!extensionsRequired.empty())
	{
		// Ошибка, не все расширения доступны
		if (!CheckInstanceExtensionsSupported(extensionsRequired)) {
			throw std::runtime_error("Vulkan: not all Extension supp");
		}

		// дополняем структуру расширениями
		instanceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();
		instanceCreateInfo.enabledExtensionCount = (uint32_t)extensionsRequired.size();

		// расширения для обработки ошибок
		bool debugReportExtensionQueried = false;

		for (const char* extensionName : extensionsRequired) {
			if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, extensionName) == 0) {
				debugReportExtensionQueried = true;
				break;
			}
		}

		//проверяем валидацию
		if (debugReportExtensionQueried && !validationLayersRequired.empty()) {

		
			if (!CheckValidationsLayersSupported(validationLayersRequired)) {
				throw std::runtime_error("Vulkan: not all validation layers supp");
			}

			
			instanceCreateInfo.enabledLayerCount = (uint32_t)validationLayersRequired.size();
			instanceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();


			validationQueried = true;
			tools::LogMessage("Vulkan: Validation layers can be created");
		}
	}

	// Хендл экземпляра
	VkInstance vkInstance;

	// Создание экземпляра
	if (vkCreateInstance(&instanceCreateInfo, nullptr, &(vkInstance)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in the 'vkCreateInstance' function.");
	}

	tools::LogMessage("Vulkan: Instance sucessfully created");

	//настройка валидации
	if (validationQueried) {

		// Конфигурация callback'а
		VkDebugReportCallbackCreateInfoEXT debugReportCallbackcreateInfo = {};
		debugReportCallbackcreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		debugReportCallbackcreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		debugReportCallbackcreateInfo.pfnCallback = VKStr::ValidationCallBack;

		// Получить адрес функции создания callback 
		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
			(PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");

		// Создать debug report callback
		if (vkCreateDebugReportCallbackEXT(vkInstance, &debugReportCallbackcreateInfo, nullptr, &(this->validationReportCallback_)) != VK_SUCCESS) {
			throw std::runtime_error("Vulkan: Error in the 'vkCreateDebugReportCallbackEXT' function. ");
		}

		tools::LogMessage("Vulkan: Report callback sucessfully created");
	}

	// Вернуть хендл instance'а vulkan'а
	return vkInstance;
}


void MainRenderer::DeinitInstance(VkInstance * vkInstance)
{
	
	if (this->validationReportCallback_ != VK_NULL_HANDLE) {

		
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
			(PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(*vkInstance, "vkDestroyDebugReportCallbackEXT");

		
		vkDestroyDebugReportCallbackEXT(*vkInstance, this->validationReportCallback_, nullptr);
		this->validationReportCallback_ = VK_NULL_HANDLE;
		tools::LogMessage("Vulkan: Report callback sucessfully destroyed");
	}

	
	if (*vkInstance != VK_NULL_HANDLE) {
		vkDestroyInstance(*vkInstance, nullptr);
		*vkInstance = VK_NULL_HANDLE;
		tools::LogMessage("Vulkan: Instance sucessfully destroyed");
	}
}

bool MainRenderer::CheckInstanceExtensionsSupported(std::vector<const char*> instanceExtensionsNames)
{


	std::vector<VkExtensionProperties> availableExtensions;

	// Получение кол-во доступных расширений
	unsigned int instanceExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);

	// Не поддерживается
	if (instanceExtensionCount == 0) {
		return false;
	}

	// Получение самих расширений, заполнение массива структур
	availableExtensions.resize(instanceExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableExtensions.data());

	//проверяем доступность
	for (const char* requiredExtName : instanceExtensionsNames) {
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