
#include "mainRenderer.h"

VKStr::Device MainRenderer::initDevice(VkInstance vkInstance, VkSurfaceKHR surface, std::vector<const char*> extensionsRequired, 
	std::vector<const char*> validationLayersRequired, bool uniqueQueueFamilies)
{
	// ���������� ������� ����� ����������
	VKStr::Device resDevice = {};

	// �������� ���-�� ��������� � �������
	unsigned int deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);

	// ���� �� ������� �����-���� ���������� � vulkan - ������
	if (deviceCount == 0) {
		throw std::runtime_error("Vulkan: Can't detect divice with Vulkan suppurt");
	}

	// �������� ��������� ���������� 
	std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
	vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physicalDevices.data());


	// �������� �� ���� �����-������ � ��������� ������ �� ������������ ����������� �����������, ���������� ������ ������������
	for (const VkPhysicalDevice& physicalDevice : physicalDevices)
	{
		// �������� ���������� �� �������� �������������� �����������
		resDevice.queueFamilies = GetQueueFamilyInfo(physicalDevice, surface, uniqueQueueFamilies);

		// ���� ������� ������� ���������� �� ���������� � ����������� - ��������� � ����������
		if (!(resDevice.queueFamilies.IsRenderingCompatible())) {
			continue;
		}

		// ���� ������ ���������� �� ������������ ������������� ���������� - ��������� � ����������
		if (extensionsRequired.size() > 0 && !CheckDeviceExtensionSupported(physicalDevice, extensionsRequired)) {
			continue;
		}

		// �������� ���������� � ��� ��� ���������� ����� �������� � ������������
		VKStr::SurfaceInfo surfaceInfo = GetSurfaceInfo(physicalDevice, surface);
		if (surfaceInfo.formats.empty() || surfaceInfo.presentModes.empty()) {
			continue;
		}

		// �������� ����� ����������� ����������, ������� ������ ��� ��������
		resDevice.physicalDevice = physicalDevice;
	}

	// ���� �� ������� �����-���� ������� ������������� ���� ����������� - ������
	if (resDevice.physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("Vulkan: Error in the 'InitDevice' function!");
	}

	// ������ �������� ��������� VkDeviceQueueCreateInfo ���������� ���������� ��� ������������� ��������
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	// ������ �������� �������� (����������� � �������������)
	uint32_t queueFamilies[2] = { (uint32_t)resDevice.queueFamilies.graphics, (uint32_t)resDevice.queueFamilies.present };

	// ���� ������. ��������� � ��������� ������������� - ���� � �� �� (��� �� ������),
	// ��� ������ ��������� ��� ������� ������ � ���� �� ���������, ����� �������� �����
	for (int i = 0; i < (uniqueQueueFamilies ? 2 : 1); i++) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamilies[i];
		queueCreateInfo.queueCount = 1;                                      // �������� ���� ������� ��� ������� ���������
		queueCreateInfo.pQueuePriorities = nullptr;                          // ������ ��-����� �������� � ����� ��������� �������� (���������� ��-����, �� ����������)
		queueCreateInfos.push_back(queueCreateInfo);                         // �������� ��������� � ������
	}


	// ���������� � ����������� ���������� ����������
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = (unsigned int)queueCreateInfos.size();

	// �������� ������������� ����������, ������� ���� ���� 
	if (!extensionsRequired.empty()) {
		if (!CheckDeviceExtensionSupported(resDevice.physicalDevice, extensionsRequired)) {
			throw std::runtime_error("Vulkan: Not all required device extensions supported.(device)");
		}

		deviceCreateInfo.enabledExtensionCount = (uint32_t)extensionsRequired.size();
		deviceCreateInfo.ppEnabledExtensionNames = extensionsRequired.data();
	}

	// �������� ������������� ����� ���������, ������� ���� ���� (���� �� �������� - ������)
	if (!validationLayersRequired.empty()) {
		if (!CheckValidationsLayersSupported(validationLayersRequired)) {
			throw std::runtime_error("Vulkan: Not all required validation layers supported. (device)");
		}

		deviceCreateInfo.enabledLayerCount = (uint32_t)validationLayersRequired.size();
		deviceCreateInfo.ppEnabledLayerNames = validationLayersRequired.data();
	}

	// ����������� ���������� 
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	// �������� ����������� ����������
	if (vkCreateDevice(resDevice.physicalDevice, &deviceCreateInfo, nullptr, &(resDevice.logicalDevice)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Failed to create logical device.");
	}

	// �������� ������ �������� ���������� (����������� ������� � ������� �������������)
	vkGetDeviceQueue(resDevice.logicalDevice, resDevice.queueFamilies.graphics, 0, &(resDevice.queues.graphics));
	vkGetDeviceQueue(resDevice.logicalDevice, resDevice.queueFamilies.present, 0, &(resDevice.queues.present));

	// ���� � ����� ���������� �� ������ - ������
	if (!resDevice.IsReady()) {
		throw std::runtime_error("Vulkan: Failed to initialize device and queues.");
	}

	// ��������� �� �������� ������������� ����������
	std::string deviceName = std::string(resDevice.GetProperties().deviceName);
	std::string message = "Vulkan: Device successfully initialized (" + deviceName + ")";
	tools::LogMessage(message);

	// ������� ����������
	return resDevice;
}

VKStr::QueueFamilyInfo MainRenderer::GetQueueFamilyInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool uniqueStrict)
{
	//������������ ������ ��������� QueueFamilyInfo � ����������� � ����������
	VKStr::QueueFamilyInfo queueFamilyInfo;

	// �������� ���-�� �������� �������� �������������� ���������� ����������� 
	unsigned int queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	// ���������������� ������ �������� �������� � �������� ��� ���������
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	// ����� �� ���������, ������� ������������ ���������� �������, � �������� ��� ������ � ������������ ������
	for (unsigned int i = 0; i < queueFamilies.size(); i++) {
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			queueFamilyInfo.graphics = i;
			break;
		}
	}

	// ����� �� ���������, ������� ������������ ������������� � �������� ��� ������ � ������������ ������
	for (unsigned int i = 0; i < queueFamilies.size(); i++) {

		// ���� ���������� ����� ID ���� ����������� 
		if (i == queueFamilyInfo.graphics && uniqueStrict) {
			continue;
		}

		// ��������� ������������� 
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
	// �������� ��������� ������ ����������� ����������
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &deviceMemoryProperties);

	// �������� �� ���� ����� � ����� ����������
	for (unsigned int i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
		if ((typeFlags & (1 << i)) && (deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	// ������������� �������� � ������ ���������� ������������ �������
	return -1;
}

bool MainRenderer::CheckDeviceExtensionSupported(VkPhysicalDevice physicalDevice, std::vector<const char*> deviceExtensionsNames)
{
	// ��������� ���������� ���������� 
	std::vector<VkExtensionProperties> availableExtensions;

	// ��������� ���-�� ��������� ����������
	unsigned int deviceExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, nullptr);

	// ���� 0 �� �� ��������������
	if (deviceExtensionCount == 0) {
		return false;
	}

	// ��������� ����� ����������
	availableExtensions.resize(deviceExtensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &deviceExtensionCount, availableExtensions.data());

	// ������ �� ���� ������������� �����������, � ��������� ���� �� ������ � ������ ���������
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