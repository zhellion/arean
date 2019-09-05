
#include "mainRenderer.h"

VkSurfaceKHR MainRenderer::initWindowSurface(VkInstance vkInstance, HINSTANCE hInstance, HWND hWnd)
{
	// Конфигурация поверхности
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKhr;
	win32SurfaceCreateInfoKhr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKhr.hwnd = hWnd;
	win32SurfaceCreateInfoKhr.hinstance = hInstance;
	win32SurfaceCreateInfoKhr.flags = 0;
	win32SurfaceCreateInfoKhr.pNext = nullptr;

	// Получить адрес функции создания поверхности для окна Windows (поскольку это функция расширения)
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
		(PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");

	// Хендл поверхности
	VkSurfaceKHR surface;

	// Инициализация (создание) поверхности
	if (vkCreateWin32SurfaceKHR(vkInstance, &win32SurfaceCreateInfoKhr, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in the 'vkCreateWin32SurfaceKHR' function");
	}

	tools::LogMessage("Vulkan: Window surface initilized");
	return surface;
}


void MainRenderer::DeinitWindowSurface(VkInstance vkInstance, VkSurfaceKHR * surface)
{
	// Если поверхность инициализирована - уничтожить
	if (surface != nullptr && *surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(vkInstance, *surface, nullptr);
		*surface = VK_NULL_HANDLE;
		tools::LogMessage("Vulkan: Window surface destroyed");
	}
}

VKStr::SurfaceInfo MainRenderer::GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	// Получить информацию о возможностях поверхности
	VKStr::SurfaceInfo surfaceInfo;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &(surfaceInfo.capabilities));

	// Получить кол-во поддерживаемых форматов
	unsigned int formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	// Если какие-либо форматы поддерживаются - получить их
	if (formatCount > 0) {
		surfaceInfo.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceInfo.formats.data());
	}

	// Получить кол-во режимов представления
	unsigned int presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	// Если какие-либо режимы представления поддерживаются - получить их
	if (presentModeCount > 0) {
		surfaceInfo.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, surfaceInfo.presentModes.data());
	}

	return surfaceInfo;
}

