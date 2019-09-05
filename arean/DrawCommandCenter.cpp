
#include "mainRenderer.h"

VkCommandPool MainRenderer::initCommandPool(const VKStr::Device &device, unsigned int queueFamilyIndex)
{
	// ���������
	VkCommandPool resultPool = VK_NULL_HANDLE;

	// �������� ����
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = (unsigned int)queueFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	// �������� ����
	if (vkCreateCommandPool(device.logicalDevice, &commandPoolCreateInfo, nullptr, &resultPool) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateCommandPool function. (Command pool)");
	}

	tools::LogMessage("Vulkan: Command pool successfully initialized");

	// ������� pool
	return resultPool;
}

void MainRenderer::DeinitCommandPool(const VKStr::Device &device, VkCommandPool * commandPool)
{
	if (commandPool != nullptr && *commandPool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(device.logicalDevice, *commandPool, nullptr);
		*commandPool = VK_NULL_HANDLE;
		tools::LogMessage("Vulkan: Command pool successfully deinitialized");
	}
}

std::vector<VkCommandBuffer> MainRenderer::initCommandBuffers(const VKStr::Device &device, VkCommandPool commandPool, unsigned int count)
{
	// ���������
	std::vector<VkCommandBuffer> resultBuffers(count);

	// ������������ ��������� �������
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;                               // �������� ���������� ����
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;                 // ���������� � ������� ���������������
	allocInfo.commandBufferCount = (unsigned int)resultBuffers.size(); // ���-�� ��������� �������

	// ������������ ������ ������
	if (vkAllocateCommandBuffers(device.logicalDevice, &allocInfo, resultBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateCommandBuffers function. (command buffer)");
	}

	tools::LogMessage("Vulkan: Command buffers successfully allocated");

	// ������� ������ �������
	return resultBuffers;
}

void MainRenderer::DeinitCommandBuffers(const VKStr::Device &device, VkCommandPool commandPool, std::vector<VkCommandBuffer> * buffers)
{
	if (device.logicalDevice != VK_NULL_HANDLE && buffers != nullptr && !buffers->empty()) {
		// ��������� ������
		vkFreeCommandBuffers(device.logicalDevice, commandPool, (unsigned int)(buffers->size()), buffers->data());
		// �������� ������
		buffers->clear();

		tools::LogMessage("Vulkan: Command buffers successfully freed");
	}
}

VKStr::Synchronization MainRenderer::InitSynchronization(const VKStr::Device & device)
{
	VKStr::Synchronization syncResult = {};

	// ���������� � ����������� �������� (������ �� ����� ���������)
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// ������� ��������� �������������
	if (vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &(syncResult.readyToRender)) != VK_SUCCESS ||
		vkCreateSemaphore(device.logicalDevice, &semaphoreInfo, nullptr, &(syncResult.readyToPresent)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error while creating synchronization primitives (synch)");
	}

	tools::LogMessage("Vulkan: Synchronization primitives sucessfully initialized");

	return syncResult;
}

void MainRenderer::DeinitSynchronization(const VKStr::Device & device, VKStr::Synchronization * sync)
{
	if (sync != nullptr) {
		if (sync->readyToRender != VK_NULL_HANDLE) {
			vkDestroySemaphore(device.logicalDevice, sync->readyToRender, nullptr);
			sync->readyToRender = VK_NULL_HANDLE;
		}

		if (sync->readyToRender != VK_NULL_HANDLE) {
			vkDestroySemaphore(device.logicalDevice, sync->readyToRender, nullptr);
			sync->readyToRender = VK_NULL_HANDLE;
		}

		tools::LogMessage("Vulkan: Synchronization primitives sucessfully deinitialized");
	}
}

void MainRenderer::PrepareDrawCommands(std::vector<VkCommandBuffer> commandBuffers, VkRenderPass renderPass, std::vector<VkPipelineLayout> pipelineLayout, VkDescriptorSet descriptorSetMain, VkDescriptorSet descriptorSetInfo, VkDescriptorSet descriptorSetVLight, const VKStr::Swapchain & swapchain, const std::vector<VKStr::Primitive>& primitives)
{
	// ���������� ������ ���������� ������
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	cmdBufInfo.pNext = nullptr;

	// ��������� ������� �������� � ������ �������
	std::vector<VkClearValue> clearValues(2);
	// ������� ������� �������� (���������)
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	// ������ ������� �������� (�������� �������-��������)
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	std::vector<VkClearValue> clearValuesShadow(1);
	clearValuesShadow[0].depthStencil = { 1.0f, 0 };
	
	VkEvent evn = {};
	for (unsigned int i = 0; i < commandBuffers.size(); ++i)
	{
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo);
	//������ ������
		if (ShadowRender)//������� ��������� ����� ����� ���� ��������� ������. ����� ������ ��� ������������ �������� ��� ��������� ����� ��� �� ����� ������������ ������ �����
		{//�������� �������� ������� - ������������ ����������� ��� ������������ �������� ������� ��������� � ������. ���������� ������ ���������� ���������, ��� �� �������� ������� ������������ ������ ���������� ����������.
			//��� �� ����� ��������� ��� �������� ������� ����������������� ��������. ����� ���������� �������. 

			renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = this->shadowPass_.renderPass;
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = this->shadowPass_.width;
			renderPassBeginInfo.renderArea.extent.height = this->shadowPass_.height;
			renderPassBeginInfo.clearValueCount = clearValuesShadow.size();
			renderPassBeginInfo.pClearValues = clearValuesShadow.data();

			renderPassBeginInfo.framebuffer = this->shadowPass_.frameBuffer;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdSetDepthBias(
				commandBuffers[i],
				1.25f,
				0.0f,
				1.75f);
			 

			if (!primitives.empty()) {
				for (unsigned int primitiveIndex = 0; primitiveIndex < primitives.size(); primitiveIndex++)
				{
					vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines_[3]);
					
					
					

					// ����� ������������ �������� ��� ������������ UBO ������� � ������� ������������
					// ��� ������ ������������� �������� ����������� �������� ��� ������������, ����� ���� ������������ ��������
					// ������� ������ UBO (� �������� ������) ��� ����������� ���������
					std::vector<uint32_t> dynamicOffsets = {
						primitiveIndex * static_cast<uint32_t>(this->device_.GetDynamicAlignment<glm::mat4>())
					};

					if (primitives_[primitiveIndex].pipelineIndex == AREAN_SHADER_T_BASE)
					{
						std::vector<VkDescriptorSet> descriptorSets = {
						this->descriptorSetShadow_
							//descriptorSetMain
						};


						vkCmdBindDescriptorSets(
							commandBuffers[i],
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipelineLayout[2],
							0,
							(uint32_t)descriptorSets.size(),
							descriptorSets.data(),
							(uint32_t)dynamicOffsets.size(),
							dynamicOffsets.data());

						// ���� ����� �������� ��������������� ���������
						if (primitives[primitiveIndex].drawIndexed && primitives[primitiveIndex].indexBuffer.count > 0) {
							// ��������� ����� ������
							VkDeviceSize offsets[1] = { 0 };
							vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

							// ��������� ����� ��������
							vkCmdBindIndexBuffer(commandBuffers[i], primitives[primitiveIndex].indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

							// ��������� ���������
							vkCmdDrawIndexed(commandBuffers[i], primitives[primitiveIndex].indexBuffer.count, 1, 0, 0, 0);
						}
						// ���� ���������� ������ �� ������������
						else {
							// ��������� ����� ������
							VkDeviceSize offsets[1] = { 0 };
							vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

							// ���������
							vkCmdDraw(commandBuffers[i], primitives[primitiveIndex].vertexBuffer.count, 1, 0, 0);
						}

					}
				}
			}

			


			vkCmdEndRenderPass(commandBuffers[i]);

	/*		VkImageSubresourceRange imageRes = {};
			imageRes.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			imageRes.baseMipLevel = 0;
			imageRes.levelCount = 1;
			imageRes.baseArrayLayer = 0;
			imageRes.layerCount = 1;

			VkImageMemoryBarrier imageMemoryBarrier = {};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.pNext = NULL;
				imageMemoryBarrier.image = this->shadowPass_.depth.image;
				imageMemoryBarrier.subresourceRange = imageRes;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			
				vkCmdPipelineBarrier(commandBuffers[i],
					VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
					VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, // srcStageMask
					VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // dstStageMask
					0,
					0, nullptr,
					0, nullptr,
					1, // imageMemoryBarrierCount
					&imageMemoryBarrier // pImageMemoryBarriers
					);*/




		}
			//������ ������

			// ���������� � ������ �������
			renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = renderPass;
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = swapchain.imageExtent.width;
			renderPassBeginInfo.renderArea.extent.height = swapchain.imageExtent.height;
			renderPassBeginInfo.clearValueCount = (uint32_t)clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();



				// ���������� ������� �����-����� (��������� �� ���-�� ����� ���-�� ��������� �������, ������� �������������)
			renderPassBeginInfo.framebuffer = swapchain.framebuffers[i];

			// ������ ������ ���-������ ��������� �������, ��� ������� ������� ��������
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);





			// �������� �� ���� ����������
			if (!primitives.empty()) {
				for (unsigned int primitiveIndex = 0; primitiveIndex < primitives.size(); primitiveIndex++)
				{

					// ��������� ����������� �������� //������ ������ ����� ��������� � ���������. ������ � ������ ��������� ���� ����� ������ ���.
					vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines_[primitives_[primitiveIndex].pipelineIndex]);



					// ����� ������������ �������� ��� ������������ UBO ������� � ������� ������������
					// ��� ������ ������������� �������� ����������� �������� ��� ������������, ����� ���� ������������ ��������
					// ������� ������ UBO (� �������� ������) ��� ����������� ���������
					std::vector<uint32_t> dynamicOffsets = {
						primitiveIndex * static_cast<uint32_t>(this->device_.GetDynamicAlignment<glm::mat4>())
					};

					// ������ ������������ (������ �������)
					// �� ��������� � ��� ������ ��������
					std::vector<VkDescriptorSet> descriptorSets = {
						descriptorSetMain
					};

					// ���� � ��������� ���� ��������
					// �������� � ������ ������������ ��� ���� (���������� �� ������ �������� � ���������� ��������� � ������)
					if (primitives[primitiveIndex].texture != nullptr) {
						descriptorSets.push_back(primitives[primitiveIndex].texture->descriptorSet);

					}

					if (primitives_[primitiveIndex].pipelineIndex != AREAN_SHADER_T_TEST_LIGHT  && this->fsInfo_.LCount > 0)
					{
						descriptorSets.push_back(descriptorSetInfo);
						descriptorSets.push_back(descriptorSetVLight);
						if (primitives_[primitiveIndex].pipelineIndex != AREAN_SHADER_T_SKY_BOX && this->ShadowRender)
						{
							descriptorSets.push_back(this->descriptorSetShadowSempler_);
						}

						// ��������� ������ ������������
						vkCmdBindDescriptorSets(
							commandBuffers[i],
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipelineLayout[0],
							0,
							(uint32_t)descriptorSets.size(),
							descriptorSets.data(),
							(uint32_t)dynamicOffsets.size(),
							dynamicOffsets.data());
					}
					else
					{
						vkCmdBindDescriptorSets(
							commandBuffers[i],
							VK_PIPELINE_BIND_POINT_GRAPHICS,
							pipelineLayout[1],
							0,
							(uint32_t)descriptorSets.size(),
							descriptorSets.data(),
							(uint32_t)dynamicOffsets.size(),
							dynamicOffsets.data());
					}




					// ���� ����� �������� ��������������� ���������
					if (primitives[primitiveIndex].drawIndexed && primitives[primitiveIndex].indexBuffer.count > 0) {
						// ��������� ����� ������
						VkDeviceSize offsets[1] = { 0 };
						vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

						// ��������� ����� ��������
						vkCmdBindIndexBuffer(commandBuffers[i], primitives[primitiveIndex].indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

						// ��������� ���������
						vkCmdDrawIndexed(commandBuffers[i], primitives[primitiveIndex].indexBuffer.count, 1, 0, 0, 0);
					}
					// ���� ���������� ������ �� ������������
					else {
						// ��������� ����� ������
						VkDeviceSize offsets[1] = { 0 };
						vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

						// ���������
						vkCmdDraw(commandBuffers[i], primitives[primitiveIndex].vertexBuffer.count, 1, 0, 0);
					}

				}
			}

			
			// ���������� �������
			vkCmdEndRenderPass(commandBuffers[i]);

			// ���������� ������� ������� ������� �������������� ������ �����-������ �
			// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ��� ������������� ����������� 

			// ���������� ������ ������
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Vulkan: Error while preparing commands (draw command)");
			}
		

		
	}
}

void MainRenderer::ResetCommandBuffers(const VKStr::Device & device, std::vector<VkCommandBuffer> commandBuffers)
{
	// ������������� ���������
	this->isReady_ = false;

	// ��������� ���������� ���� ������ � ��������
	if (device.queues.graphics != VK_NULL_HANDLE && device.queues.present != VK_NULL_HANDLE) {
		vkQueueWaitIdle(device.queues.graphics);
		vkQueueWaitIdle(device.queues.present);
	}

	// �������� ������ ������
	if (!commandBuffers.empty()) {
		for (const VkCommandBuffer &buffer : commandBuffers) {
			if (vkResetCommandBuffer(buffer, 0) != VK_SUCCESS) {
				throw std::runtime_error("Vulkan: Error while resetting commad buffers");
			}
		}
	}

	// ����� ������ � ����������
	this->isReady_ = true;
}

VkCommandBuffer MainRenderer::CreateSingleTimeCommandBuffer(const VKStr::Device &device, VkCommandPool commandPool)
{
	
	VkCommandBuffer commandBuffer;

	// ������������ �����
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(device.logicalDevice, &allocInfo, &commandBuffer);

	// ������ ��������� ����� (����� � ������ ������)
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // ���������� ���� ��� � ������� ����������
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	
	return commandBuffer;
}

void MainRenderer::FlushSingleTimeCommandBuffer(const VKStr::Device &device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
{
	// ��������� ���������� ���������� �������
	vkEndCommandBuffer(commandBuffer);

	// �������� ������ � �������
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

	// ������� ���������� �������
	vkQueueWaitIdle(queue);

	// ������� �����
	vkFreeCommandBuffers(device.logicalDevice, commandPool, 1, &commandBuffer);
}
