
#include "mainRenderer.h"

//-----------------------------------------------------
//����� �������
void MainRenderer::FreeUboModels(VKStr::UboModelArray * uboModels)
{
	_aligned_free(*uboModels);
	*uboModels = nullptr;
	tools::LogMessage("Vulkan: Dynamic UBO satage-buffer successfully freed");
}

VKStr::Buffer MainRenderer::CreateBuffer(const VKStr::Device & device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
	VkSharingMode sharingMode)
{
	VKStr::Buffer resultBuffer;

	// ���������� ������
	resultBuffer.size = size;

	// ��������� �������� vk-������
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = sharingMode;
	bufferInfo.flags = 0;

	// ������� �������� ������
	if (vkCreateBuffer(device.logicalDevice, &bufferInfo, nullptr, &(resultBuffer.vkBuffer)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating buffer. (CreateBuffer)");
	}

	// �������� ���������� ������ � ������
	VkMemoryRequirements memRequirements = {};
	vkGetBufferMemoryRequirements(device.logicalDevice, resultBuffer.vkBuffer, &memRequirements);

	// �������� ������ ���� ������ ���������������� ����������� ������
	int memoryTypeIndex = GetMemoryTypeIndex(device.physicalDevice, memRequirements.memoryTypeBits, properties);
	if (memoryTypeIndex < 0) {
		throw std::runtime_error("Vulkan: Can't find suitable memory type! (create buffer)");
	}

	// ��������� ��������� ������ (�������� ���������� � ���������� ������)
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = (unsigned int)memoryTypeIndex;

	// ��������� ������ ��� ������
	if (vkAllocateMemory(device.logicalDevice, &memoryAllocateInfo, nullptr, &(resultBuffer.vkDeviceMemory)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while allocating buffer memory! (create buffer)");
	}

	// ��������� ������ � ������
	vkBindBufferMemory(device.logicalDevice, resultBuffer.vkBuffer, resultBuffer.vkDeviceMemory, 0);


	return resultBuffer;
}

VKStr::UniformBuffer MainRenderer::initStandartUnformBuffer(const VKStr::Device &device, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{

	VKStr::UniformBuffer resultBuffer = {};

	// ������� �����, �������� ������, ��������� ������ � ������
	VKStr::Buffer buffer = CreateBuffer(
		device,
		bufferSize,
		usage,
		properties);

	// �������� ������������ �������������� ������
	resultBuffer.vkBuffer = buffer.vkBuffer;
	resultBuffer.vkDeviceMemory = buffer.vkDeviceMemory;
	resultBuffer.size = buffer.size;

	// ��������� ���������� ��� �����������
	resultBuffer.configDescriptorInfo(buffer.size, 0);

	// ��������� ����� (������� ��� ��������� ��� ����������� ����������)
	resultBuffer.map(device.logicalDevice, buffer.size, 0);

	tools::LogMessage("Vulkan: Uniform buffer scene successfully allocated");

	return resultBuffer;
}

void MainRenderer::DeinitUniformBuffer(const VKStr::Device & device, VKStr::UniformBuffer * uniformBuffer)
{
	if (uniformBuffer != nullptr) {

		uniformBuffer->unmap(device.logicalDevice);

		if (uniformBuffer->vkBuffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device.logicalDevice, uniformBuffer->vkBuffer, nullptr);
			uniformBuffer->vkBuffer = VK_NULL_HANDLE;
		}

		if (uniformBuffer->vkDeviceMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device.logicalDevice, uniformBuffer->vkDeviceMemory, nullptr);
			uniformBuffer->vkDeviceMemory = VK_NULL_HANDLE;
		}

		uniformBuffer->descriptorBufferInfo = {};
		*uniformBuffer = {};

		tools::LogMessage("Vulkan: Uniform buffer successfully deinitialized");
	}
}

VKStr::UboModelArray MainRenderer::AllocateUboModels(const VKStr::Device & device, unsigned int maxObjects)
{
	// �������� ����������� ������������ ��� ���� glm::mat4
	std::size_t dynamicAlignment = (std::size_t)device.GetDynamicAlignment<glm::mat4>();

	// ��������� ������ ������ �������� ��������� ������������ ������ (��� ���� glm::mat4 �������� � 64 �����)
	std::size_t bufferSize = (std::size_t)(dynamicAlignment * maxObjects);

	// ������������ ������ � ������ ������������
	VKStr::UboModelArray result = (VKStr::UboModelArray)_aligned_malloc(bufferSize, dynamicAlignment);

	tools::LogMessage("Vulkan: Dynamic UBO satage-buffer successfully allocated");

	return result;
}