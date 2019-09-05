
#include "mainRenderer.h"


//----------------------------------------------------------------
//����� �������
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
//�������
VkSampler MainRenderer::initTextureSampler(const VKStr::Device & device)
{

	VkSampler resultSampler;

	// ��������� ��������
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;                      // ��� ������������ ����� ������� ������ ����������
	samplerInfo.minFilter = VK_FILTER_LINEAR;                      // ��� ������������ ����� ������� ������ ����������
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;     // ��������� ��� ������ �� �������
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;                        // ������� ������������ ����������
	samplerInfo.maxAnisotropy = 16;                                 // ������� ����������
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;    // ���� �����
	samplerInfo.unnormalizedCoordinates = VK_FALSE;                // ������������ ��������������� ���������� (�� ����������)
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	// �������� ��������
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

	// ��������� �������� ����
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// ���� ���������� ��� ����������� ��������
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER , 1 },
	};

	// ������������ ����
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = maxDescriptorSets;

	// �������� �������������� ����
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
			0,                                            // ������ ��������
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,    // ��� ����������� (������� �����������)
			1,                                            // ���-�� ������������
			VK_SHADER_STAGE_FRAGMENT_BIT,                 // ���� ��������� (���������� ������)
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

	// �������� ����� ����� ������������ �� ��������������� ����
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


	// ������������ ����������� � ����� ������������
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // ��� ���������
			nullptr,                                     // pNext
			descriptorSetResult,						// ������� ����� ������������
			0,                                           // ����� �������� (� �������)
			0,                                           // ������� ������ (������ �� ������������)
			1,                                           // ���-�� ������������
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,   // ��� �����������
			&imageInfo,                                  // ���������� � ���������� �����������
			nullptr,
			nullptr
		}
	};

	// �������� ������ ������������
	vkUpdateDescriptorSets(this->device_.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);


	tools::LogMessage("Vulkan: Descriptor set successfully initialized");

	// ������� ����� ������
	return descriptorSetResult;
}

//-------------------------------------------------------------------------
//���������� �������� ���� �������
VkDescriptorPool MainRenderer::initDescriptorPoolMain(const VKStr::Device &device)
{

	VkDescriptorPool descriptorPoolResult = VK_NULL_HANDLE;

	// ��������� �������� ����
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// ���� ���������� ��� ����������� uniform-������
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER , 1 },
		// ���� ���������� ��� unform-������� ��������� �������� (������������)
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }
	};


	// ������������ ����
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = 1;

	// �������� �������������� ����
	if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool");
	}

	tools::LogMessage("Vulkan: Main descriptor pool successfully initialized");


	return descriptorPoolResult;
}

VkDescriptorSetLayout MainRenderer::initDescriptorSetLayoutMain(const VKStr::Device & device)
{

	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;

	// �������� ������������ � ������ ���������

	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // ������ ��������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // ��� ����������� (����� �����, �������)
			1,                                            // ���-�� ������������
			VK_SHADER_STAGE_VERTEX_BIT,                   // ���� ��������� (��������� ������)
			nullptr
		},
		{
			1,                                            // ������ ��������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    // ��� ����������� (����� �����, ������������)
			1,                                            // ���-�� ������������
			VK_SHADER_STAGE_VERTEX_BIT| VK_SHADER_STAGE_FRAGMENT_BIT,                   // ���� ��������� (��������� ������)
			nullptr
		}
	};

	// ���������������� ���������� �������������� ������
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

	// �������� ������������ � ������ ���������

	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // ������ ��������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            // ��� ����������� (����� �����, �������)
			1,                                            // ���-�� ������������
			VK_SHADER_STAGE_VERTEX_BIT,                   // ���� ��������� (��������� ������)
			nullptr
		},
		{
			1,                                            // ������ ��������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    // ��� ����������� (����� �����, ������������)
			1,                                            // ���-�� ������������
			VK_SHADER_STAGE_VERTEX_BIT,                   // ���� ��������� (��������� ������)
			nullptr
		}
	};

	// ���������������� ���������� �������������� ������
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

	// �������� ����� ����� ������������ �� ��������������� ����
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocInfo, &descriptorSetResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
	}

	// ������������ ����������� � ����� ������������
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // ��� ���������
			nullptr,                                     // pNext
			descriptorSetResult,                         // ������� ����� ������������
			0,                                           // ����� �������� (� �������)
			0,                                           // ������� ������ (������ �� ������������)
			1,                                           // ���-�� ������������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,           // ��� �����������
			nullptr,
			&(uniformBufferWorld.descriptorBufferInfo),  // ���������� � ���������� ������
			nullptr
		},
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // ��� ���������
			nullptr,                                     // pNext
			descriptorSetResult,                         // ������� ����� ������������
			1,                                           // ����� �������� (� �������)
			0,                                           // ������� ������ (������ �� ������������)
			1,                                           // ���-�� ������������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,   // ��� �����������
			nullptr,
			&(uniformBufferModels.descriptorBufferInfo), // ���������� � ���������� ������
			nullptr,
		},
	};

	// �������� ������ ������������
	vkUpdateDescriptorSets(device.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

	tools::LogMessage("Vulkan: Descriptor set successfully initialized");

	// ������� ����� ������
	return descriptorSetResult;
}

//------------------------------------------------------------------
//����������� ���� ������� ���������� �����

VkDescriptorPool MainRenderer::initDescriptorPoolLight(const VKStr::Device &device)
{

	VkDescriptorPool descriptorPoolResult = VK_NULL_HANDLE;

	// ��������� �������� ����
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// ���� ���������� ��� ����������� uniform-������
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		
	};


	// ������������ ����
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = 1;

	// �������� �������������� ����
	if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool (Light)");
	}

	tools::LogMessage("Vulkan: Main descriptor pool successfully initialized");


	return descriptorPoolResult;
}

VkDescriptorSetLayout MainRenderer::initDescriptorSetLayoutLight(const VKStr::Device & device)
{

	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;

	// �������� ������������ � ������ ���������

	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // ������ ��������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,                                            // ���-�� ������������
			VK_SHADER_STAGE_FRAGMENT_BIT,                   // ���� ��������� (��������� ������)
			nullptr
		}
	};

	// ���������������� ���������� �������������� ������
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

	// �������� ����� ����� ������������ �� ��������������� ����
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocInfo, &descriptorSetResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
	}

	// ������������ ����������� � ����� ������������
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // ��� ���������
			nullptr,                                     // pNext
			descriptorSetResult,                         // ������� ����� ������������
			0,                                           // ����� �������� (� �������)
			0,                                           // ������� ������ (������ �� ������������)
			1,                                           // ���-�� ������������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,           // ��� �����������
			nullptr,
			&(uniformBufferLCount.descriptorBufferInfo),  // ���������� � ���������� ������
			nullptr
		}
	};

	// �������� ������ ������������
	vkUpdateDescriptorSets(device.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

	tools::LogMessage("Vulkan: Descriptor set successfully initialized");

	// ������� ����� ������
	return descriptorSetResult;
}
//--------------------------------------------------------
//������� �����
VkDescriptorPool MainRenderer::initDescriptorPoolVLight(const VKStr::Device &device)
{

	VkDescriptorPool descriptorPoolResult = VK_NULL_HANDLE;

	// ��������� �������� ����
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		// ���� ���������� ��� ����������� uniform-������
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },

	};


	// ������������ ����
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = (uint32_t)descriptorPoolSizes.size();
	poolInfo.pPoolSizes = descriptorPoolSizes.data();
	poolInfo.maxSets = 1;

	// �������� �������������� ����
	if (vkCreateDescriptorPool(device.logicalDevice, &poolInfo, nullptr, &descriptorPoolResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateDescriptorPool function. Cant't create descriptor pool (Light)");
	}

	tools::LogMessage("Vulkan: Main descriptor pool successfully initialized");


	return descriptorPoolResult;
}

VkDescriptorSetLayout MainRenderer::initDescriptorSetLayoutVLight(const VKStr::Device & device)
{

	VkDescriptorSetLayout layoutResult = VK_NULL_HANDLE;

	// �������� ������������ � ������ ���������

	std::vector<VkDescriptorSetLayoutBinding> bindings =
	{
		{
			0,                                            // ������ ��������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,                                            // ���-�� ������������
			VK_SHADER_STAGE_VERTEX_BIT |VK_SHADER_STAGE_FRAGMENT_BIT,                   // ���� ��������� (��������� ������)
			nullptr
		}
	};

	// ���������������� ���������� �������������� ������
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

	// �������� ����� ����� ������������ �� ��������������� ����
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = descriptorPool;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayout;

	if (vkAllocateDescriptorSets(device.logicalDevice, &descriptorSetAllocInfo, &descriptorSetResult) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set");
	}

	// ������������ ����������� � ����� ������������
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // ��� ���������
			nullptr,                                     // pNext
			descriptorSetResult,                         // ������� ����� ������������
			0,                                           // ����� �������� (� �������)
			0,                                           // ������� ������ (������ �� ������������)
			1,                                           // ���-�� ������������
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,			// ��� �����������
			nullptr,
			&(uniformBufferVectorsLight.descriptorBufferInfo), // ���������� � ���������� ������
			nullptr,
		}
	};

	// �������� ������ ������������
	vkUpdateDescriptorSets(device.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

	tools::LogMessage("Vulkan: Descriptor set successfully initialized");

	// ������� ����� ������
	return descriptorSetResult;

}
