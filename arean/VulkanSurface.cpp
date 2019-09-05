
#include "mainRenderer.h"

VkSurfaceKHR MainRenderer::initWindowSurface(VkInstance vkInstance, HINSTANCE hInstance, HWND hWnd)
{
	// ������������ �����������
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKhr;
	win32SurfaceCreateInfoKhr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKhr.hwnd = hWnd;
	win32SurfaceCreateInfoKhr.hinstance = hInstance;
	win32SurfaceCreateInfoKhr.flags = 0;
	win32SurfaceCreateInfoKhr.pNext = nullptr;

	// �������� ����� ������� �������� ����������� ��� ���� Windows (��������� ��� ������� ����������)
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR =
		(PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(vkInstance, "vkCreateWin32SurfaceKHR");

	// ����� �����������
	VkSurfaceKHR surface;

	// ������������� (��������) �����������
	if (vkCreateWin32SurfaceKHR(vkInstance, &win32SurfaceCreateInfoKhr, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in the 'vkCreateWin32SurfaceKHR' function");
	}

	tools::LogMessage("Vulkan: Window surface initilized");
	return surface;
}


void MainRenderer::DeinitWindowSurface(VkInstance vkInstance, VkSurfaceKHR * surface)
{
	// ���� ����������� ���������������� - ����������
	if (surface != nullptr && *surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(vkInstance, *surface, nullptr);
		*surface = VK_NULL_HANDLE;
		tools::LogMessage("Vulkan: Window surface destroyed");
	}
}

VKStr::SurfaceInfo MainRenderer::GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	// �������� ���������� � ������������ �����������
	VKStr::SurfaceInfo surfaceInfo;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &(surfaceInfo.capabilities));

	// �������� ���-�� �������������� ��������
	unsigned int formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

	// ���� �����-���� ������� �������������� - �������� ��
	if (formatCount > 0) {
		surfaceInfo.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceInfo.formats.data());
	}

	// �������� ���-�� ������� �������������
	unsigned int presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	// ���� �����-���� ������ ������������� �������������� - �������� ��
	if (presentModeCount > 0) {
		surfaceInfo.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, surfaceInfo.presentModes.data());
	}

	return surfaceInfo;
}

