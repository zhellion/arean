
#include "mainRenderer.h"

void MainRenderer::Pause()
{
	// Ожидание завершения всех возможных процессов
	if (this->device_.logicalDevice != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(this->device_.logicalDevice);
	}

	this->isRendering_ = false;
}

void MainRenderer::Continue()
{
	this->isRendering_ = true;
}

void MainRenderer::VideoSettingsChanged()
{
	
	this->Pause();

	// В начале деинициализировать компоненты зависимые от swap-chain
	this->DeinitCommandBuffers(this->device_, this->commandPoolDraw_, &(this->commandBuffersDraw_));
	for (size_t i = 0; i < this->pipelines_.size(); i++)
	{
		this->DeinitGraphicsPipeline(this->device_, &(this->pipelines_[i]));
		
	}

	// Render pass не зависит от swap-chain, но поскольку поверхность могла сменить свои свойства - следует пересоздать
	// по новой, проверив формат цветового вложения
	this->DeinitRenderPassage(this->device_, &(this->renderPass_));
	this->renderPass_ = this->initRenderPassage(this->device_, this->surface_, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);

	// Ре-инициализация swap-cahin. 
	// В начале получаем старый swap-chain
	VKStr::Swapchain oldSwapChain = this->swapchain_;
	// Инициализируем обновленный
	this->swapchain_ = this->initSwapChain(
		this->device_,
		this->surface_,
		{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		this->renderPass_,
		3,
		&oldSwapChain);
	// Уничтожаем старый
	this->DeinitSwapchain(this->device_, &(oldSwapChain));
	if (fsInfo_.LCount == 0)
	{
		// Инициализация графического конвейера
		this->pipelines_[0] = this->initGraphicsPipeline(this->device_, this->pipelineLayout_[1], this->swapchain_, this->renderPass_, "baseNoLight.frag.spv", "baseNoLight.vert.spv");
		this->pipelines_[1] = this->initSBGraphicsPipeline(this->device_, this->pipelineLayout_[1], this->swapchain_, this->renderPass_, "baseNoLight.frag.spv", "baseNoLight.vert.spv");
		this->ShadowRender = false;
	}
	else
	{
		this->pipelines_[0] = this->initGraphicsPipeline(this->device_, this->pipelineLayout_[0], this->swapchain_, this->renderPass_, "base.frag.spv", "base.vert.spv");
		this->pipelines_[1] = this->initSBGraphicsPipeline(this->device_, this->pipelineLayout_[0], this->swapchain_, this->renderPass_, "sb.frag.spv", "sb.vert.spv");
		this->ShadowRender = true;
	}
	this->pipelines_[2] = this->initGraphicsPipeline(this->device_, this->pipelineLayout_[1], this->swapchain_, this->renderPass_, "baseTestLight.frag.spv", "baseTestLight.vert.spv");
	this->pipelines_[3] =(this->initShadowPipeline(this->device_, this->pipelineLayout_[2], this->shadowPass_.renderPass, "ShadowMap.vert.spv"));

	// Аллокация командных буферов (получение хендлов)
	this->commandBuffersDraw_ = this->initCommandBuffers(this->device_, this->commandPoolDraw_, (unsigned int)this->swapchain_.framebuffers.size());


	// Подготовка базовых комманд
	this->PrepareDrawCommands(
		this->commandBuffersDraw_,
		this->renderPass_,
		this->pipelineLayout_,
		this->descriptorSetMain_,
		this->descriptorSetInfo_,
		this->descriptorSetVLight_,
		this->swapchain_,
		this->primitives_);


	
	this->Continue();

	
	this->Update();
}

void MainRenderer::Draw()
{

	if (!this->isReady_ || !this->isRendering_) {
		return;
	}

	// Индекс доступного изображения
	unsigned int imageIndex;

	// Получить индекс доступного изображения из swap-chain и "включить" семафор сигнализирующий о доступности изображения для рендеринга
	VkResult acquireStatus = vkAcquireNextImageKHR(
		this->device_.logicalDevice,
		this->swapchain_.vkSwapchain,
		10000,
		this->sync_.readyToRender,
		VK_NULL_HANDLE,
		&imageIndex);

	// Если не получилось получить изображение, вероятно поверхность изменилась или swap-chain более ей не соответствует по каким-либо причинам
	// VK_SUBOPTIMAL_KHR означает что swap-chain еще может быть использован, но в полной мере поверхности не соответствует
	if (acquireStatus != VK_SUCCESS && acquireStatus != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Vulkan: Error. Can't acquire swap-chain image (Draw)");
	}

	// Данные семафоры будут ожидаться на определенных стадиях ковейера
	std::vector<VkSemaphore> waitSemaphores = { this->sync_.readyToRender };

	// Данные семафоры будут "включаться" на определенных стадиях ковейера
	std::vector<VkSemaphore> signalSemaphores = { this->sync_.readyToPresent };

	// Стадии конвейера на которых будет происходить ожидание семафоров (на i-ой стадии включения i-ого семафора из waitSemaphores)		
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	// Информация об отправке команд в буфер
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();       // Кол-во семафоров ожидания
	submitInfo.pWaitSemaphores = waitSemaphores.data();                    // Семафоры велючение которых будет ожидаться
	submitInfo.pWaitDstStageMask = waitStages;                             // Стадии на которых конвейер "приостановиться" до включения семафоров
	submitInfo.commandBufferCount = 1;                                     // Число командных буферов за одну отправку
	submitInfo.pCommandBuffers = &(this->commandBuffersDraw_[imageIndex]) ; // Командный буфер (для текущего изображения в swap-chain)
	submitInfo.signalSemaphoreCount = (uint32_t)signalSemaphores.size();   // Кол-во семафоров сигнала (завершения стадии)
	submitInfo.pSignalSemaphores = signalSemaphores.data();                // Семафоры которые включатся при завершении


	// Инициировать отправку команд в очередь (на рендеринг) 
	VkResult result = vkQueueSubmit(this->device_.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error. Can't submit commands (draw)"); //ошибка семплинга отлавлвается здесь.
	}
	
	// Настройка представления 
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = (uint32_t)signalSemaphores.size();    // Кол-во ожидаемых семафоров
	presentInfo.pWaitSemaphores = signalSemaphores.data();                 // Cемафоры "включение" которых ожидается перед показом
	presentInfo.swapchainCount = 1;                                        // Кол-во swap-chain'ов
	presentInfo.pSwapchains = &(this->swapchain_.vkSwapchain);             // Указание текущего swap-chain
	presentInfo.pImageIndices = &imageIndex;                               // Индекс текущего изображения, куда осуществляется показ
	presentInfo.pResults = nullptr;

	// Инициировать представление
	VkResult presentStatus = vkQueuePresentKHR(this->device_.queues.present, &presentInfo);

	// Представление могло не выполниться если поверхность изменилась или swap-chain более ей не соответствует
	if (presentStatus != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error. Failed to present! (draw)");
	}
}

void MainRenderer::Update() //переписать, иначе камера будет прыгать к центру, что не очень
{	
	//-------------------------------------------------------
	//для проверки теней от 1 источника света
	this->uboVectorsLight_[0].LightProjection = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 500.0f);
	this->uboVectorsLight_[0].LightProjection[1][1] *= -1;
	this->uboVectorsLight_[0].LightView = glm::lookAt(glm::vec3(
		uboVectorsLight_[0].outVec1.x,
		uboVectorsLight_[0].outVec1.y,
		uboVectorsLight_[0].outVec1.z
	),
	this->SelectedObject,
	glm::vec3(0, 1, 0));
	this->uboVectorsLight_[0].LightWorld = glm::mat4();
	VKStr::UboWorld a;
	a.projectionMatrix = this->uboVectorsLight_[0].LightProjection;
	a.viewMatrix = this->uboVectorsLight_[0].LightView;
	a.worldMatrix = this->uboVectorsLight_[0].LightWorld;
	memcpy(this->uniformBufferVectorsLight_.pMapped, &(this->uboVectorsLight_), (size_t)(this->uniformBufferVectorsLight_.size));
	memcpy(this->ShadowBuff_.pMapped, &a, (size_t)(this->ShadowBuff_.size));
	//-------------------------------------------------------

	// Соотношение сторон (используем размеры поверхности определенные при создании swap-chain)
	this->camera_.aspectRatio = (float)(this->swapchain_.imageExtent.width) / (float)(this->swapchain_.imageExtent.height);

	// Настройка матрицы проекции
	// При помощи данной матрицы происходит проекция 3-мерных точек на плоскость
	// Считается что наблюдатель (камера) в центре системы координат
	this->uboWorld_.projectionMatrix = this->camera_.MakeProjectionMatrix();

	// Настройка матрицы вида
	// Отвечает за положение и поворот камеры (по сути приводит систему координат мира к системе координат наблюдателя)
	this->uboWorld_.viewMatrix = this->camera_.MakeViewMatrix();

	// Матрица модели мира
	// Позволяет осуществлять глобальные преобразования всей сцены (пока что не используется)
	this->uboWorld_.worldMatrix = glm::mat4();


	// Копировать данные в uniform-буфер
	memcpy(this->uniformBufferWorld_.pMapped, &(this->uboWorld_), (size_t)(this->uniformBufferWorld_.size));

	// Теперь необходимо обновить динамический буфер формы объектов (если они есть)
	if (!this->primitives_.empty()) {

		// Динамическое выравнивание для одного элемента массива
		VkDeviceSize dynamicAlignment = this->device_.GetDynamicAlignment<glm::mat4>();

		// Пройтись по всем объектам
		for (unsigned int i = 0; i < this->primitives_.size(); i++) {

			// Используя выравнивание получить указатель на нужный элемент массива
			glm::mat4* modelMat = (glm::mat4*)(((uint64_t)(this->uboModels_) + (i * dynamicAlignment)));

			// Вписать данные матрицы в элемент
			*modelMat = glm::translate(glm::mat4(), this->primitives_[i].position);
			*modelMat = glm::rotate(*modelMat, glm::radians(this->primitives_[i].rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			*modelMat = glm::rotate(*modelMat, glm::radians(this->primitives_[i].rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			*modelMat = glm::rotate(*modelMat, glm::radians(this->primitives_[i].rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			*modelMat = glm::scale(*modelMat, this->primitives_[i].scale);
		}


		// Копировать данные в uniform-буфер
		memcpy(this->uniformBufferModels_.pMapped, this->uboModels_, (size_t)(this->uniformBufferModels_.size));

		// Гарантировать видимость обновленной памяти устройством
		VkMappedMemoryRange memoryRange = {};
		memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		memoryRange.memory = this->uniformBufferModels_.vkDeviceMemory;
		memoryRange.size = this->uniformBufferModels_.size;
		vkFlushMappedMemoryRanges(this->device_.logicalDevice, 1, &memoryRange);

		
	}
	//memcpy(this->uniformBufferLCount_.pMapped, &this->lightInfo_, sizeof(VKStr::LightInfo));

}

void MainRenderer::SetCameraPerspectiveSettings(float fFOV, float fNear, float fFar)
{
	this->camera_.fFOV = fFOV;
	this->camera_.fNear = fNear;
	this->camera_.fFar = fFar;
}

void MainRenderer::SetCameraPosition(float x, float y, float z)
{
	this->camera_.position = glm::vec3(x, y, z);
	outCamPos.x = x;
	outCamPos.y =  y;
	outCamPos.z = z;
	this->fsInfo_.CamPos = glm::vec4(-x, -y, -z, 0.0f);
	memcpy(this->uniformBufferInfo_.pMapped, &(this->fsInfo_), (size_t)(this->uniformBufferInfo_.size));
}

void MainRenderer::SetCameraRotation(float x, float y, float z)
{
	this->camera_.roation = glm::vec3(x, y, z);
}

glm::vec3 MainRenderer::CameraPos()
{
	return this->outCamPos;
}

unsigned int MainRenderer::AddPrimitive(const std::vector<VKStr::Vertex>& vertices, const std::vector<unsigned int>& indices, 
	const VKStr::Texture * texture, int pipelineIndex, glm::vec3 position, glm::vec3 rotaton, glm::vec3 scale)
{
	VKStr::Primitive primitive;
	primitive.position = position;
	primitive.rotation = rotaton;
	primitive.scale = scale;
	primitive.texture = texture;
	primitive.drawIndexed = !indices.empty();
	primitive.pipelineIndex = pipelineIndex;
	// Буфер вершин (временный)
	std::vector<VKStr::Vertex> vertexBuffer = vertices;
	VkDeviceSize vertexBufferSize = ((unsigned int)vertexBuffer.size()) * sizeof(VKStr::Vertex);
	unsigned int vertexCount = (unsigned int)vertexBuffer.size();

	// Создать буфер вершин в памяти хоста
	VKStr::Buffer tmp = CreateBuffer(this->device_, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	primitive.vertexBuffer.vkBuffer = tmp.vkBuffer;
	primitive.vertexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
	primitive.vertexBuffer.size = tmp.size;
	primitive.vertexBuffer.count = vertexCount;

	// Разметить память буфера вершин и скопировать в него данные, после чего убрать разметку
	void * verticesMemPtr;
	vkMapMemory(this->device_.logicalDevice, primitive.vertexBuffer.vkDeviceMemory, 0, vertexBufferSize, 0, &verticesMemPtr);
	memcpy(verticesMemPtr, vertexBuffer.data(), (std::size_t)vertexBufferSize);
	vkUnmapMemory(this->device_.logicalDevice, primitive.vertexBuffer.vkDeviceMemory);

	// Если необходимо рисовать индексированную геометрию
	if (primitive.drawIndexed) {

		// Буфер индексов (временный)
		std::vector<unsigned int> indexBuffer = indices;
		VkDeviceSize indexBufferSize = ((unsigned int)indexBuffer.size()) * sizeof(unsigned int);
		unsigned int indexCount = (unsigned int)indexBuffer.size();

		// Cоздать буфер индексов в памяти хоста
		tmp = CreateBuffer(this->device_, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		primitive.indexBuffer.vkBuffer = tmp.vkBuffer;
		primitive.indexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
		primitive.indexBuffer.size = tmp.size;
		primitive.indexBuffer.count = indexCount;

		// Разметить память буфера индексов и скопировать в него данные, после чего убрать разметку
		void * indicesMemPtr;
		vkMapMemory(this->device_.logicalDevice, primitive.indexBuffer.vkDeviceMemory, 0, indexBufferSize, 0, &indicesMemPtr);
		memcpy(indicesMemPtr, indexBuffer.data(), (std::size_t)indexBufferSize);
		vkUnmapMemory(this->device_.logicalDevice, primitive.indexBuffer.vkDeviceMemory);
	}

		// Впихнуть новый примитив в массив
		this->primitives_.push_back(primitive);

		// Обновить командный буфер
		this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
		this->PrepareDrawCommands(this->commandBuffersDraw_, this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_, this->swapchain_, this->primitives_);

		// Вернуть индекс
		return (unsigned int)(this->primitives_.size() - 1);
	
	
}

//обновление примитива
void MainRenderer::UpdatePrimitive(unsigned int primitieInc, glm::vec3 position, glm::vec3 rotaton, glm::vec3 scale)
{
	VKStr::Primitive primitive;
	primitive = this->primitives_[primitieInc];
	primitive.position = position;
	primitive.rotation = rotaton;
	primitive.scale = scale;
	
	// Впихнуть новый примитив в массив
	this->primitives_[primitieInc]=primitive;

	// Обновить командный буфер
	this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
	this->PrepareDrawCommands(this->commandBuffersDraw_, 
		this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_,
		this->swapchain_, this->primitives_);
}

//инициализация света (данные, которые будут передаваться в шейдер примитивов, для перераспределения яркостей
unsigned int MainRenderer::initLight(glm::vec3 position, glm::vec3 LightColor, float ambientRate, float diffuseRate, float specularRate, float constant, float linear, float quadratic)
{
	if (fsInfo_.LCount > 128)
	{
		tools::LogMessage("Light massiv is full, cant add new light");
		return -1;
	}
	else
	{
		glm::vec3 ambient = LightColor * ambientRate;
		glm::vec3 diffuse = LightColor * diffuseRate;
		glm::vec3 specular = LightColor * specularRate;
		
		this->uboVectorsLight_[fsInfo_.LCount].outVec1 = glm::vec4(position, ambient.x); // position.xyz + ambient.x
		this->uboVectorsLight_[fsInfo_.LCount].outVec2 = glm::vec4(ambient.y, ambient.z, diffuse.x, diffuse.y); //ambient.yz + diffuse.xy
		this->uboVectorsLight_[fsInfo_.LCount].outvec3 = glm::vec4(diffuse.z, specular); //diffuse.z + specular.xyz
		this->uboVectorsLight_[fsInfo_.LCount].outvec4 = glm::vec4(constant, linear, quadratic, ambientRate);//последнее - смещение на 4 бита. туда можно поместить 1 константу типа float


		this->fsInfo_.LCount++;


		
		
		if (fsInfo_.LCount == 1)
		{
			this->pipelines_[0] = this->initGraphicsPipeline(this->device_, this->pipelineLayout_[0], this->swapchain_, this->renderPass_, "base.frag.spv", "base.vert.spv");
			this->pipelines_[1] = this->initSBGraphicsPipeline(this->device_, this->pipelineLayout_[0], this->swapchain_, this->renderPass_, "sb.frag.spv", "sb.vert.spv");
			this->ShadowRender = true;
		}
		return fsInfo_.LCount - 1;
		this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
		this->PrepareDrawCommands(this->commandBuffersDraw_,
			this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_,
			this->swapchain_, this->primitives_);

	}
	this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
	this->PrepareDrawCommands(this->commandBuffersDraw_, this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_, this->swapchain_, this->primitives_);
}

void MainRenderer::reInitLight(glm::vec3 position, int IDLight)
{

		this->uboVectorsLight_[IDLight].outVec1 = glm::vec4(position, this->uboVectorsLight_[IDLight].outVec1.w); // position.xyz + ambient.x
		this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
		this->PrepareDrawCommands(this->commandBuffersDraw_,
			this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_,
			this->swapchain_, this->primitives_);

		this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
		this->PrepareDrawCommands(this->commandBuffersDraw_, this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_, this->swapchain_, this->primitives_);
}

void MainRenderer::DeleteLight(int IDLight)
{
	for (size_t i = IDLight; i < fsInfo_.LCount-1; i++)
	{
		this->uboVectorsLight_[fsInfo_.LCount] = this->uboVectorsLight_[fsInfo_.LCount + 1];
		
	}
	if (fsInfo_.LCount == 0)
	{
		this->pipelines_[0] = this->initGraphicsPipeline(this->device_, this->pipelineLayout_[1], this->swapchain_, this->renderPass_, "baseNoLight.frag.spv", "baseNoLight.vert.spv");
		this->pipelines_[1] = this->initSBGraphicsPipeline(this->device_, this->pipelineLayout_[1], this->swapchain_, this->renderPass_, "baseNoLight.frag.spv", "baseNoLight.vert.spv");
		this->ShadowRender = false;
	}
	fsInfo_.LCount--;
	this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
	this->PrepareDrawCommands(this->commandBuffersDraw_,
		this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_,
		this->swapchain_, this->primitives_);

	this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
	this->PrepareDrawCommands(this->commandBuffersDraw_, this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_, this->swapchain_, this->primitives_);
}


VKStr::Texture MainRenderer::CreateTexture(const unsigned char * pixels, uint32_t width, uint32_t height, uint32_t channels, uint32_t bpp)
{
	// Приостановить выполнение основных команд (если какие-либо в процессе)
	this->Pause();

	
	VKStr::Texture resultTexture = {};

	// Размер изображения (ожидаем по умолчанию 4 байта на пиксель, в режиме RGBA)
	VkDeviceSize size = (VkDeviceSize)(width * height * bpp);

	// Если данных не обнаружено
	if (!pixels) {
		throw std::runtime_error("Vulkan: Error while creating texture. Empty pixel buffer recieved (createTexture)");
	}

	// Создать промежуточное изображение
	VKStr::Image stagingImage = CreateImageSingle(
		this->device_,
		VK_IMAGE_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		{ width,height,1 },
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		VK_IMAGE_TILING_LINEAR, 
		VK_SAMPLE_COUNT_1_BIT 
		);

	// Выбрать подресурс изображения (мип-уровень 0, слой - 0)
	VkImageSubresource subresource = {};
	subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource.mipLevel = 0;
	subresource.arrayLayer = 0;

	// Размещение байт в подресурсе
	VkSubresourceLayout stagingImageSubresourceLayout = {};
	vkGetImageSubresourceLayout(this->device_.logicalDevice, stagingImage.vkImage, &subresource, &stagingImageSubresourceLayout);

	// Разметить память под изображение
	void* data;
	vkMapMemory(this->device_.logicalDevice, stagingImage.vkDeviceMemory, 0, size, 0, &data);

	// Если "ширина строки" равна кол-ву пиксилей по ширине помноженному на bpp - можно исользовать обычный memcpy
	if (stagingImageSubresourceLayout.rowPitch == width * bpp) {
		memcpy(data, pixels, (unsigned int)size);
	}
	// Если нет (например размер изображения не кратен степени двойки) - перебираем байты со смещением и копируем каждую стороку
	else {
		unsigned char* dataBytes = reinterpret_cast<unsigned char*>(data);
		for (unsigned int y = 0; y < height; y++) {
			memcpy(
				&dataBytes[y * (stagingImageSubresourceLayout.rowPitch)],
				&pixels[y * width * bpp],
				width * bpp
			);
		}
	}

	// Убрать разметку памяти
	vkUnmapMemory(this->device_.logicalDevice, stagingImage.vkDeviceMemory);

	// Создать финальное изображение (в памяти устройства)
	resultTexture.image = CreateImageSingle(
		this->device_,
		VK_IMAGE_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		{ width,height,1 },
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_TILING_OPTIMAL, 
		VK_SAMPLE_COUNT_1_BIT);


	// Создать командный буфер для команд перевода размещения изображений
	VkCommandBuffer transitionCmdBuffer = CreateSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_);

	// Подресурс подвергающийся смене размещения в изображениях (описываем его)
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	// Сменить размещение памяти промежуточного изображения в VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	CmdImageLayoutTransition(transitionCmdBuffer, stagingImage.vkImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

	// Сменить размещение памяти целевого изображения в VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	CmdImageLayoutTransition(transitionCmdBuffer, resultTexture.image.vkImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

	// Выполнить команды перевода размещения
	FlushSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_, transitionCmdBuffer, this->device_.queues.graphics);

	// Создать командный буфер для копирования изображения
	VkCommandBuffer copyCmdBuffer = CreateSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_);

	// Копирование из промежуточной картинки в основную
	CmdImageCopy(copyCmdBuffer, stagingImage.vkImage, resultTexture.image.vkImage, width, height);

	// Выполнить команды копирования
	FlushSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_, copyCmdBuffer, this->device_.queues.graphics);

	// Очистить промежуточное изображение
	stagingImage.Deinit(this->device_.logicalDevice);


	// Получить новый набор дескрипторов из дескриптороного пула
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = this->descriptorPoolTextures_;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &(this->descriptorSetLayoutTextures_);

	if (vkAllocateDescriptorSets(this->device_.logicalDevice, &descriptorSetAllocInfo, &(resultTexture.descriptorSet)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set for texture (createTexture)");
	}

	// Информация о передаваемом изображении
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = resultTexture.image.vkImageView;
	imageInfo.sampler = this->textureSampler_;

	// Конфигурация добавляемых в набор дескрипторов
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // Тип структуры
			nullptr,                                     // pNext
			resultTexture.descriptorSet,                 // Целевой набор дескрипторов
			0,                                           // Точка привязки (у шейдера)
			0,                                           // Элемент массив (массив не используется)
			1,                                           // Кол-во дескрипторов
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,   // Тип дескриптора
			&imageInfo,                                  // Информация о параметрах изображения
			nullptr,
			nullptr
		}
	};

	// Обновить наборы дескрипторов
	vkUpdateDescriptorSets(this->device_.logicalDevice, (uint32_t)writes.size(), writes.data(), 0, nullptr);

	// Исполнение основых команд снова возможно
	this->Continue();

	
	return resultTexture;
}

MainRenderer::MainRenderer(HINSTANCE hInstance, HWND hWnd, unsigned int primitivesMaxCount):
	isReady_(false),
	isRendering_(true),
	instance_(VK_NULL_HANDLE),
	validationReportCallback_(VK_NULL_HANDLE),
	surface_(VK_NULL_HANDLE),
	renderPass_(VK_NULL_HANDLE),
	commandPoolDraw_(VK_NULL_HANDLE),
	descriptorSetLayoutMain_(VK_NULL_HANDLE),
	descriptorSetLayoutTextures_(VK_NULL_HANDLE),
	descriptorSetMain_(VK_NULL_HANDLE),
	descriptorSetInfo_(VK_NULL_HANDLE),
	descriptorSetLayoutInfo_(VK_NULL_HANDLE),
	pipelineLayout_(VK_NULL_HANDLE),
	pipelines_(VK_NULL_HANDLE),
	primitivesMaxCount_(primitivesMaxCount),
	uboModels_(nullptr),
	outCamPos(0.0f, 0.0f, -1.0f)
{
	// Присвоить параметры камеры по умолчанию
	this->camera_.fFar = DEFAULT_FOV;
	this->camera_.fFar = DEFAULT_FAR;
	this->camera_.fNear = DEFAULT_NEAR;

	// Списки расширений и слоев запрашиваемых по умолчанию
	std::vector<const char*> instanceExtensionsRequired = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
	std::vector<const char*> deviceExtensionsRequired = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<const char*> validationLayersRequired = {};

	// Если это DEBUG конфигурация - запросить еще расширения и слои для валидации
	if (IS_VK_DEBUG) {
		instanceExtensionsRequired.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		validationLayersRequired.push_back("VK_LAYER_LUNARG_standard_validation");
	}

	// Инициализация экземпляра
	this->instance_ = this->initInstance("vulkan is trash", "and this engine is trash", instanceExtensionsRequired, validationLayersRequired);

	// Инициализация поверхности отображения
	this->surface_ = this->initWindowSurface(this->instance_, hInstance, hWnd);

	// Инициализация устройства
	this->device_ = this->initDevice(this->instance_, this->surface_, deviceExtensionsRequired, validationLayersRequired, false);
	// Инициализация прохода рендеринга
	this->renderPass_ = this->initRenderPassage(this->device_, this->surface_, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);
	this->shadowPass_.renderPass = this->initShadowRenderPassage(this->device_);
	this->initShadowFrameBuffer(this->device_);

	// Инициализация swap-chain
	this->swapchain_ = this->initSwapChain(this->device_, this->surface_, { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_FORMAT_D32_SFLOAT_S8_UINT, this->renderPass_, 3, nullptr);

	// Инициализация командного пула
	this->commandPoolDraw_ = this->initCommandPool(this->device_, this->device_.queueFamilies.graphics);

	// Аллокация командных буферов (получение хендлов)
	this->commandBuffersDraw_ = this->initCommandBuffers(this->device_, this->commandPoolDraw_, (unsigned int)(this->swapchain_.framebuffers.size()));

	// Аллокация глобального uniform-буфера
	this->uniformBufferWorld_ = this->initStandartUnformBuffer(this->device_, 
																sizeof(VKStr::UboWorld),
																VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Аллокация uniform-буфера отдельных объектов (динамический буфер)
	this->uniformBufferModels_ = this->initStandartUnformBuffer(this->device_, 
																this->device_.GetDynamicAlignment<glm::mat4>() * this->primitivesMaxCount_,
																VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// Создание дескрипторного пула для выделения основного набора (для unform-буфера)
	this->descriptorPoolMain_ = this->initDescriptorPoolMain(this->device_);

	// Создание дескрипторного пула для выделения текстурного набора (текстурные семплеры)
	this->descriptorPoolTextures_ = this->initDescriptorPoolTextures(this->device_);

	// Инициализация размещения основного дескрипторного набора
	this->descriptorSetLayoutMain_ = this->initDescriptorSetLayoutMain(this->device_);

	// Инициализация размещения теккстурного набора
	this->descriptorSetLayoutTextures_ = this->initDescriptorSetLayoutTextures(this->device_);

	

	// Инициализация текстурного семплера
	this->textureSampler_ = this->initTextureSampler(this->device_);

	// Инициализация дескрипторного набора
	this->descriptorSetMain_ = this->initDescriptorSetMain(
		this->device_,
		this->descriptorPoolMain_,
		this->descriptorSetLayoutMain_,
		this->uniformBufferWorld_,
		this->uniformBufferModels_);

	//для источников света
	this->uniformBufferInfo_ = this->initStandartUnformBuffer(this->device_,
																sizeof(VKStr::FSInfo),
																VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	this->uniformBufferVectorsLight_ = this->initStandartUnformBuffer(this->device_, 
																	sizeof(VKStr::UBOVectorsLight)*128,
																	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	//для подпрохода рендера теней
	this->ShadowBuff_ = this->initStandartUnformBuffer(this->device_,
						sizeof(VKStr::UboWorld),
						VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	

	this->descriptorPoolInfo_ = this->initDescriptorPoolLight(this->device_);
	this->descriptorSetLayoutInfo_ = this->initDescriptorSetLayoutLight(this->device_);

	this->descriptorSetInfo_ = this->initDescriptorSetLight(this->device_, this->descriptorPoolInfo_, this->descriptorSetLayoutInfo_, this->uniformBufferInfo_);


	this->descriptorPoolVLight_ = this->initDescriptorPoolVLight(this->device_);
	this->descriptorSetLayoutVLight_ = this->initDescriptorSetLayoutVLight(this->device_);

	this->descriptorSetVLight_ = this->initDescriptorSetVLight(this->device_, this->descriptorPoolVLight_, this->descriptorSetLayoutVLight_, this->uniformBufferVectorsLight_);

	this->descriptorPoolShadow_ = this->initDescriptorPoolMain(this->device_);
	this->descriptorSetLayoutShadow_ = this->initShadowDescriptorSetLayoutMain(this->device_);



	this->descriptorSetShadow_ = this->initDescriptorSetMain(
		this->device_,
		this->descriptorPoolShadow_,
		this->descriptorSetLayoutMain_,
		this->ShadowBuff_,
		this->uniformBufferModels_);

	//для вывода в текстурный семплер
	this->descriptorPoolShadowSempler_ = this->initDescriptorPoolTextures(this->device_);
	this->descriptorSetLayoutShadowSempler_ = this->initDescriptorSetLayoutTextures(this->device_);
	this->descriptorSetShadowSempler_ = this->initShadowSamplerDescriptorSet(this->device_, this->descriptorPoolShadowSempler_, this->descriptorSetLayoutShadowSempler_, this->shadowPass_);

	// Аллокация памяти массива ubo-объектов отдельных примитивов
	this->uboModels_ = this->AllocateUboModels(this->device_, this->primitivesMaxCount_);

	// Инициализация размещения графического конвейера
	this->pipelineLayout_.push_back(this->initPipelineLayout(this->device_, { this->descriptorSetLayoutMain_, this->descriptorSetLayoutTextures_, this->descriptorSetLayoutInfo_, descriptorSetLayoutVLight_, this->descriptorSetLayoutShadowSempler_ }));
	this->pipelineLayout_.push_back(this->initPipelineLayout(this->device_, { this->descriptorSetLayoutMain_, this->descriptorSetLayoutTextures_}));
	this->pipelineLayout_.push_back(this->initPipelineLayout(this->device_, { this->descriptorSetLayoutShadow_ }));

	if (fsInfo_.LCount == 0)
	{
		this->pipelines_.push_back(this->initGraphicsPipeline(this->device_, this->pipelineLayout_[1], this->swapchain_, this->renderPass_, "baseNoLight.frag.spv", "baseNoLight.vert.spv"));
		this->pipelines_.push_back(this->initSBGraphicsPipeline(this->device_, this->pipelineLayout_[1], this->swapchain_, this->renderPass_, "baseNoLight.frag.spv", "baseNoLight.vert.spv"));
	}
	else
	{
		// Инициализация графического конвейера
		this->pipelines_.push_back(this->initGraphicsPipeline(this->device_, this->pipelineLayout_[0], this->swapchain_, this->renderPass_, "base.frag.spv", "base.vert.spv"));
		this->pipelines_.push_back(this->initSBGraphicsPipeline(this->device_, this->pipelineLayout_[0], this->swapchain_, this->renderPass_, "sb.frag.spv", "sb.vert.spv"));
		this->ShadowRender = true;
	}
	this->pipelines_.push_back( this->initGraphicsPipeline(this->device_, this->pipelineLayout_[1], this->swapchain_, this->renderPass_, "baseTestLight.frag.spv", "baseTestLight.vert.spv"));
	this->pipelines_.push_back(this->initShadowPipeline(this->device_, this->pipelineLayout_[2], this->shadowPass_.renderPass, "ShadowMap.vert.spv"));


	// Примитивы синхронизации
	this->sync_ = this->InitSynchronization(this->device_);

	// Подготовка базовых комманд
	this->PrepareDrawCommands(
		this->commandBuffersDraw_,
		this->renderPass_,
		this->pipelineLayout_,
		this->descriptorSetMain_,
		this->descriptorSetInfo_,
		this->descriptorSetVLight_,
		this->swapchain_,
		this->primitives_);


	// Готово к рендерингу
	this->isReady_ = true;

	// Обновить
	this->Update();
	
}

	MainRenderer::~MainRenderer()
	{
		this->Pause();
		this->isReady_ = false;

		// Сброс буферов команд
		this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);

		// Деинициализация примитивов синхронизации
		this->DeinitSynchronization(this->device_, &(this->sync_));

		// Деинициализация графического конвейера
		for (size_t i = 0; i < this->pipelines_.size(); i++)
		{
			this->DeinitGraphicsPipeline(this->device_, &(this->pipelines_[i]));
			pipelines_.pop_back();
		}

		// Деинициализация графического конвейера
		for (size_t i = 0; i<this->pipelineLayout_.size(); i++)
			this->DeinitPipelineLayout(this->device_, &(this->pipelineLayout_[i]));

		// Очистка памяти массива ubo-объектов отдельных примитивов
		this->FreeUboModels(&(this->uboModels_));

		// Деинициализация набора дескрипторов
		this->DeinitDescriptorSet(this->device_, this->descriptorPoolMain_, &(this->descriptorSetMain_));

		// Деинициализация текстурного семплера
		this->DeinitTextureSampler(this->device_, &(this->textureSampler_));

		this->DeinitDescriptorSet(this->device_, this->descriptorPoolInfo_, &(this->descriptorSetInfo_));


		this->DeinitDescriptorSet(this->device_, this->descriptorPoolInfo_, &(this->descriptorSetVLight_));

		this->DeinitDescriptorSet(this->device_, this->descriptorPoolInfo_, &(this->descriptorSetShadow_));

		// Деинициализация размещения текстурного дескрипторного набора
		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutTextures_));

		// Деинициализация размещения основоного дескрипторного набора
		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutMain_));


		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutVLight_));
		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutShadowSempler_));

		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutShadow_));

		// Уничтожение ntrcnehyjuj дескрипторного пула
		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolTextures_));

		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolShadowSempler_));

		// Уничтожение основного дескрипторного пула
		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolMain_));

		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolInfo_));


		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolVLight_));

		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolShadow_));

		// Деинициализация uniform-буффера объектов
		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferModels_));

		// Деинициализация глобального uniform-буффера
		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferWorld_));

		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferVectorsLight_));


		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferInfo_));

		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferVectorsLight_));

		this->DeinitUniformBuffer(this->device_, &(this->ShadowBuff_));

		// Деинициализация командных буферов
		this->DeinitCommandBuffers(this->device_, this->commandPoolDraw_, &(this->commandBuffersDraw_));

		// Деинициализация командного пула
		this->DeinitCommandPool(this->device_, &(this->commandPoolDraw_));

		// Деинициализация swap-chain'а
		this->DeinitSwapchain(this->device_, &(this->swapchain_));

		// Деинциализация прохода рендеринга
		this->DeinitRenderPassage(this->device_, &(this->renderPass_));
		this->DeinitRenderPassage(this->device_, &(this->shadowPass_.renderPass));

		// Динициализация устройства
		this->DeinitDevice(&(this->device_));

		// Деинициализация поверзности
		this->DeinitWindowSurface(this->instance_, &(this->surface_));

		// Деинициализация экземпляра Vulkan
		this->DeinitInstance(&(this->instance_));
	}


