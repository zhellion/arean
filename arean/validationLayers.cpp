#include "mainRenderer.h"

bool MainRenderer::CheckValidationsLayersSupported(std::vector<const char*> validationLayersNames)
{

	std::vector<VkLayerProperties> availableLayers;

	// Получение кол-ва доступных слоев
	unsigned int layersCount = 0;
	vkEnumerateInstanceLayerProperties(&layersCount, nullptr);


	if (layersCount == 0) {
		return false;
	}

	// Получение самих слоев, заполнение массива
	availableLayers.resize(layersCount);
	vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data());

	// проверка поддержки слоев
	for (const char* requiredName : validationLayersNames) {
		bool found = false;
		for (const VkLayerProperties &properties : availableLayers) {
			if (strcmp(requiredName, properties.layerName) == 0) {
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


VKAPI_ATTR VkBool32 VKAPI_CALL VKStr::ValidationCallBack(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj,
	size_t location,
	int32_t code,
	const char* layerPrefix,
	const char* msg,
	void* userData)
{
	std::string message;
	message.append("Validation layer : ");
	message.append(msg);
	tools::LogMessage(message);
	return VK_FALSE;
}
