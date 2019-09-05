
#include "mainRenderer.h"

VkCommandPool MainRenderer::initCommandPool(const VKStr::Device &device, unsigned int queueFamilyIndex)
{
	// Результат
	VkCommandPool resultPool = VK_NULL_HANDLE;

	// Описание пула
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = (unsigned int)queueFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	// Создание пула
	if (vkCreateCommandPool(device.logicalDevice, &commandPoolCreateInfo, nullptr, &resultPool) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkCreateCommandPool function. (Command pool)");
	}

	tools::LogMessage("Vulkan: Command pool successfully initialized");

	// Вернуть pool
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
	// Результат
	std::vector<VkCommandBuffer> resultBuffers(count);

	// Конфигурация аллокации буферов
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;                               // Указание командного пула
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;                 // Передается в очередь непосредственно
	allocInfo.commandBufferCount = (unsigned int)resultBuffers.size(); // Кол-во командных буферов

	// Аллоцировать буферы команд
	if (vkAllocateCommandBuffers(device.logicalDevice, &allocInfo, resultBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateCommandBuffers function. (command buffer)");
	}

	tools::LogMessage("Vulkan: Command buffers successfully allocated");

	// Вернуть массив хендлов
	return resultBuffers;
}

void MainRenderer::DeinitCommandBuffers(const VKStr::Device &device, VkCommandPool commandPool, std::vector<VkCommandBuffer> * buffers)
{
	if (device.logicalDevice != VK_NULL_HANDLE && buffers != nullptr && !buffers->empty()) {
		// Очистисть память
		vkFreeCommandBuffers(device.logicalDevice, commandPool, (unsigned int)(buffers->size()), buffers->data());
		// Очистить массив
		buffers->clear();

		tools::LogMessage("Vulkan: Command buffers successfully freed");
	}
}

VKStr::Synchronization MainRenderer::InitSynchronization(const VKStr::Device & device)
{
	VKStr::Synchronization syncResult = {};

	// Информация о создаваемом семафоре (ничего не нужно указывать)
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Создать примитивы синхронизации
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
	// Информация начала командного буфера
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	cmdBufInfo.pNext = nullptr;

	// Параметры очистки вложений в начале прохода
	std::vector<VkClearValue> clearValues(2);
	// Очистка первого вложения (цветового)
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	// Очиска второго вложения (вложения глубины-трфарета)
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	std::vector<VkClearValue> clearValuesShadow(1);
	clearValuesShadow[0].depthStencil = { 1.0f, 0 };
	
	VkEvent evn = {};
	for (unsigned int i = 0; i < commandBuffers.size(); ++i)
	{
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		vkBeginCommandBuffer(commandBuffers[i], &cmdBufInfo);
	//первый проход
		if (ShadowRender)//каждому источнику света нужен свой отдельный проход. Нужно понять как подгружаются семплеры для генерации теней или же можно использовать каскад теней
		{//проблема теневого прохода - неправильные дискрипторы или некорректная передача мировых координат в шейдер. координаты модели передаются нормально, так же проверка обычным дескриптором выдала адекватные результаты.
			//так же нужно проверить как работает алгорит перераспределения яркостей. Буфер достаточно большой. 

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
					
					
					

					// Спиок динамических смещений для динамических UBO буферов в наборах дескрипторов
					// При помощи выравниваниях получаем необходимое смещение для дескрипторов, чтобы была осуществлена привязка
					// нужного буфера UBO (с матрицей модели) для конкретного примитива
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

						// Если нужно рисовать индексированную геометрию
						if (primitives[primitiveIndex].drawIndexed && primitives[primitiveIndex].indexBuffer.count > 0) {
							// Привязать буфер вершин
							VkDeviceSize offsets[1] = { 0 };
							vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

							// Привязать буфер индексов
							vkCmdBindIndexBuffer(commandBuffers[i], primitives[primitiveIndex].indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

							// Отрисовка геометрии
							vkCmdDrawIndexed(commandBuffers[i], primitives[primitiveIndex].indexBuffer.count, 1, 0, 0, 0);
						}
						// Если индексация вершин не используется
						else {
							// Привязать буфер вершин
							VkDeviceSize offsets[1] = { 0 };
							vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

							// Отрисовка
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
			//второй проход

			// Информация о начале прохода
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



				// Установить целевой фрейм-буфер (поскольку их кол-во равно кол-ву командных буферов, индексы соответствуют)
			renderPassBeginInfo.framebuffer = swapchain.framebuffers[i];

			// Начать первый под-проход основного прохода, это очистит цветоые вложения
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);





			// Пройтись по всем примитивам
			if (!primitives.empty()) {
				for (unsigned int primitiveIndex = 0; primitiveIndex < primitives.size(); primitiveIndex++)
				{

					// Привязать графический конвейер //теперь шейдер можно привязать к примитиву. Значит в классе примитива надо будет задать это.
					vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelines_[primitives_[primitiveIndex].pipelineIndex]);



					// Спиок динамических смещений для динамических UBO буферов в наборах дескрипторов
					// При помощи выравниваниях получаем необходимое смещение для дескрипторов, чтобы была осуществлена привязка
					// нужного буфера UBO (с матрицей модели) для конкретного примитива
					std::vector<uint32_t> dynamicOffsets = {
						primitiveIndex * static_cast<uint32_t>(this->device_.GetDynamicAlignment<glm::mat4>())
					};

					// Наборы дескрипторов (массив наборов)
					// По умолчанию в нем только основной
					std::vector<VkDescriptorSet> descriptorSets = {
						descriptorSetMain
					};

					// Если у примитива есть текстура
					// Добавить в список дескрипторов еще один (отвечающий за подачу текстуры и параметров семплинга в шейдер)
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

						// Привязать наборы дескрипторов
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




					// Если нужно рисовать индексированную геометрию
					if (primitives[primitiveIndex].drawIndexed && primitives[primitiveIndex].indexBuffer.count > 0) {
						// Привязать буфер вершин
						VkDeviceSize offsets[1] = { 0 };
						vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

						// Привязать буфер индексов
						vkCmdBindIndexBuffer(commandBuffers[i], primitives[primitiveIndex].indexBuffer.vkBuffer, 0, VK_INDEX_TYPE_UINT32);

						// Отрисовка геометрии
						vkCmdDrawIndexed(commandBuffers[i], primitives[primitiveIndex].indexBuffer.count, 1, 0, 0, 0);
					}
					// Если индексация вершин не используется
					else {
						// Привязать буфер вершин
						VkDeviceSize offsets[1] = { 0 };
						vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, &(primitives[primitiveIndex].vertexBuffer.vkBuffer), offsets);

						// Отрисовка
						vkCmdDraw(commandBuffers[i], primitives[primitiveIndex].vertexBuffer.count, 1, 0, 0);
					}

				}
			}

			
			// Завершение прохода
			vkCmdEndRenderPass(commandBuffers[i]);

			// Завершение прохода добавит неявное преобразование памяти фрейм-буфера в
			// VK_IMAGE_LAYOUT_PRESENT_SRC_KHR для представления содержимого 

			// Завершение записи команд
			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Vulkan: Error while preparing commands (draw command)");
			}
		

		
	}
}

void MainRenderer::ResetCommandBuffers(const VKStr::Device & device, std::vector<VkCommandBuffer> commandBuffers)
{
	// Приостановить рендеринг
	this->isReady_ = false;

	// Подождать завершения всех команд в очередях
	if (device.queues.graphics != VK_NULL_HANDLE && device.queues.present != VK_NULL_HANDLE) {
		vkQueueWaitIdle(device.queues.graphics);
		vkQueueWaitIdle(device.queues.present);
	}

	// Сбросить буферы команд
	if (!commandBuffers.empty()) {
		for (const VkCommandBuffer &buffer : commandBuffers) {
			if (vkResetCommandBuffer(buffer, 0) != VK_SUCCESS) {
				throw std::runtime_error("Vulkan: Error while resetting commad buffers");
			}
		}
	}

	// Снова готово к рендерингу
	this->isReady_ = true;
}

VkCommandBuffer MainRenderer::CreateSingleTimeCommandBuffer(const VKStr::Device &device, VkCommandPool commandPool)
{
	
	VkCommandBuffer commandBuffer;

	// Аллоцировать буфер
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(device.logicalDevice, &allocInfo, &commandBuffer);

	// Начать командный буфер (готов к записи команд)
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Используем один раз и ожидаем результата
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	
	return commandBuffer;
}

void MainRenderer::FlushSingleTimeCommandBuffer(const VKStr::Device &device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue)
{
	// Завершаем наполнение командного буффера
	vkEndCommandBuffer(commandBuffer);

	// Отправка команд в очередь
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

	// Ожидаем выполнения команды
	vkQueueWaitIdle(queue);

	// Очищаем буфер
	vkFreeCommandBuffers(device.logicalDevice, commandPool, 1, &commandBuffer);
}
