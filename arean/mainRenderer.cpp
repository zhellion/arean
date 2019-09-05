
#include "mainRenderer.h"

void MainRenderer::Pause()
{
	// �������� ���������� ���� ��������� ���������
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

	// � ������ ������������������ ���������� ��������� �� swap-chain
	this->DeinitCommandBuffers(this->device_, this->commandPoolDraw_, &(this->commandBuffersDraw_));
	for (size_t i = 0; i < this->pipelines_.size(); i++)
	{
		this->DeinitGraphicsPipeline(this->device_, &(this->pipelines_[i]));
		
	}

	// Render pass �� ������� �� swap-chain, �� ��������� ����������� ����� ������� ���� �������� - ������� �����������
	// �� �����, �������� ������ ��������� ��������
	this->DeinitRenderPassage(this->device_, &(this->renderPass_));
	this->renderPass_ = this->initRenderPassage(this->device_, this->surface_, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);

	// ��-������������� swap-cahin. 
	// � ������ �������� ������ swap-chain
	VKStr::Swapchain oldSwapChain = this->swapchain_;
	// �������������� �����������
	this->swapchain_ = this->initSwapChain(
		this->device_,
		this->surface_,
		{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		this->renderPass_,
		3,
		&oldSwapChain);
	// ���������� ������
	this->DeinitSwapchain(this->device_, &(oldSwapChain));
	if (fsInfo_.LCount == 0)
	{
		// ������������� ������������ ���������
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

	// ��������� ��������� ������� (��������� �������)
	this->commandBuffersDraw_ = this->initCommandBuffers(this->device_, this->commandPoolDraw_, (unsigned int)this->swapchain_.framebuffers.size());


	// ���������� ������� �������
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

	// ������ ���������� �����������
	unsigned int imageIndex;

	// �������� ������ ���������� ����������� �� swap-chain � "��������" ������� ��������������� � ����������� ����������� ��� ����������
	VkResult acquireStatus = vkAcquireNextImageKHR(
		this->device_.logicalDevice,
		this->swapchain_.vkSwapchain,
		10000,
		this->sync_.readyToRender,
		VK_NULL_HANDLE,
		&imageIndex);

	// ���� �� ���������� �������� �����������, �������� ����������� ���������� ��� swap-chain ����� �� �� ������������� �� �����-���� ��������
	// VK_SUBOPTIMAL_KHR �������� ��� swap-chain ��� ����� ���� �����������, �� � ������ ���� ����������� �� �������������
	if (acquireStatus != VK_SUCCESS && acquireStatus != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Vulkan: Error. Can't acquire swap-chain image (Draw)");
	}

	// ������ �������� ����� ��������� �� ������������ ������� ��������
	std::vector<VkSemaphore> waitSemaphores = { this->sync_.readyToRender };

	// ������ �������� ����� "����������" �� ������������ ������� ��������
	std::vector<VkSemaphore> signalSemaphores = { this->sync_.readyToPresent };

	// ������ ��������� �� ������� ����� ����������� �������� ��������� (�� i-�� ������ ��������� i-��� �������� �� waitSemaphores)		
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	// ���������� �� �������� ������ � �����
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = (uint32_t)waitSemaphores.size();       // ���-�� ��������� ��������
	submitInfo.pWaitSemaphores = waitSemaphores.data();                    // �������� ��������� ������� ����� ���������
	submitInfo.pWaitDstStageMask = waitStages;                             // ������ �� ������� �������� "���������������" �� ��������� ���������
	submitInfo.commandBufferCount = 1;                                     // ����� ��������� ������� �� ���� ��������
	submitInfo.pCommandBuffers = &(this->commandBuffersDraw_[imageIndex]) ; // ��������� ����� (��� �������� ����������� � swap-chain)
	submitInfo.signalSemaphoreCount = (uint32_t)signalSemaphores.size();   // ���-�� ��������� ������� (���������� ������)
	submitInfo.pSignalSemaphores = signalSemaphores.data();                // �������� ������� ��������� ��� ����������


	// ������������ �������� ������ � ������� (�� ���������) 
	VkResult result = vkQueueSubmit(this->device_.queues.graphics, 1, &submitInfo, VK_NULL_HANDLE);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error. Can't submit commands (draw)"); //������ ��������� ������������ �����.
	}
	
	// ��������� ������������� 
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = (uint32_t)signalSemaphores.size();    // ���-�� ��������� ���������
	presentInfo.pWaitSemaphores = signalSemaphores.data();                 // C������� "���������" ������� ��������� ����� �������
	presentInfo.swapchainCount = 1;                                        // ���-�� swap-chain'��
	presentInfo.pSwapchains = &(this->swapchain_.vkSwapchain);             // �������� �������� swap-chain
	presentInfo.pImageIndices = &imageIndex;                               // ������ �������� �����������, ���� �������������� �����
	presentInfo.pResults = nullptr;

	// ������������ �������������
	VkResult presentStatus = vkQueuePresentKHR(this->device_.queues.present, &presentInfo);

	// ������������� ����� �� ����������� ���� ����������� ���������� ��� swap-chain ����� �� �� �������������
	if (presentStatus != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error. Failed to present! (draw)");
	}
}

void MainRenderer::Update() //����������, ����� ������ ����� ������� � ������, ��� �� �����
{	
	//-------------------------------------------------------
	//��� �������� ����� �� 1 ��������� �����
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

	// ����������� ������ (���������� ������� ����������� ������������ ��� �������� swap-chain)
	this->camera_.aspectRatio = (float)(this->swapchain_.imageExtent.width) / (float)(this->swapchain_.imageExtent.height);

	// ��������� ������� ��������
	// ��� ������ ������ ������� ���������� �������� 3-������ ����� �� ���������
	// ��������� ��� ����������� (������) � ������ ������� ���������
	this->uboWorld_.projectionMatrix = this->camera_.MakeProjectionMatrix();

	// ��������� ������� ����
	// �������� �� ��������� � ������� ������ (�� ���� �������� ������� ��������� ���� � ������� ��������� �����������)
	this->uboWorld_.viewMatrix = this->camera_.MakeViewMatrix();

	// ������� ������ ����
	// ��������� ������������ ���������� �������������� ���� ����� (���� ��� �� ������������)
	this->uboWorld_.worldMatrix = glm::mat4();


	// ���������� ������ � uniform-�����
	memcpy(this->uniformBufferWorld_.pMapped, &(this->uboWorld_), (size_t)(this->uniformBufferWorld_.size));

	// ������ ���������� �������� ������������ ����� ����� �������� (���� ��� ����)
	if (!this->primitives_.empty()) {

		// ������������ ������������ ��� ������ �������� �������
		VkDeviceSize dynamicAlignment = this->device_.GetDynamicAlignment<glm::mat4>();

		// �������� �� ���� ��������
		for (unsigned int i = 0; i < this->primitives_.size(); i++) {

			// ��������� ������������ �������� ��������� �� ������ ������� �������
			glm::mat4* modelMat = (glm::mat4*)(((uint64_t)(this->uboModels_) + (i * dynamicAlignment)));

			// ������� ������ ������� � �������
			*modelMat = glm::translate(glm::mat4(), this->primitives_[i].position);
			*modelMat = glm::rotate(*modelMat, glm::radians(this->primitives_[i].rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			*modelMat = glm::rotate(*modelMat, glm::radians(this->primitives_[i].rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			*modelMat = glm::rotate(*modelMat, glm::radians(this->primitives_[i].rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
			*modelMat = glm::scale(*modelMat, this->primitives_[i].scale);
		}


		// ���������� ������ � uniform-�����
		memcpy(this->uniformBufferModels_.pMapped, this->uboModels_, (size_t)(this->uniformBufferModels_.size));

		// ������������� ��������� ����������� ������ �����������
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
	// ����� ������ (���������)
	std::vector<VKStr::Vertex> vertexBuffer = vertices;
	VkDeviceSize vertexBufferSize = ((unsigned int)vertexBuffer.size()) * sizeof(VKStr::Vertex);
	unsigned int vertexCount = (unsigned int)vertexBuffer.size();

	// ������� ����� ������ � ������ �����
	VKStr::Buffer tmp = CreateBuffer(this->device_, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	primitive.vertexBuffer.vkBuffer = tmp.vkBuffer;
	primitive.vertexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
	primitive.vertexBuffer.size = tmp.size;
	primitive.vertexBuffer.count = vertexCount;

	// ��������� ������ ������ ������ � ����������� � ���� ������, ����� ���� ������ ��������
	void * verticesMemPtr;
	vkMapMemory(this->device_.logicalDevice, primitive.vertexBuffer.vkDeviceMemory, 0, vertexBufferSize, 0, &verticesMemPtr);
	memcpy(verticesMemPtr, vertexBuffer.data(), (std::size_t)vertexBufferSize);
	vkUnmapMemory(this->device_.logicalDevice, primitive.vertexBuffer.vkDeviceMemory);

	// ���� ���������� �������� ��������������� ���������
	if (primitive.drawIndexed) {

		// ����� �������� (���������)
		std::vector<unsigned int> indexBuffer = indices;
		VkDeviceSize indexBufferSize = ((unsigned int)indexBuffer.size()) * sizeof(unsigned int);
		unsigned int indexCount = (unsigned int)indexBuffer.size();

		// C������ ����� �������� � ������ �����
		tmp = CreateBuffer(this->device_, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		primitive.indexBuffer.vkBuffer = tmp.vkBuffer;
		primitive.indexBuffer.vkDeviceMemory = tmp.vkDeviceMemory;
		primitive.indexBuffer.size = tmp.size;
		primitive.indexBuffer.count = indexCount;

		// ��������� ������ ������ �������� � ����������� � ���� ������, ����� ���� ������ ��������
		void * indicesMemPtr;
		vkMapMemory(this->device_.logicalDevice, primitive.indexBuffer.vkDeviceMemory, 0, indexBufferSize, 0, &indicesMemPtr);
		memcpy(indicesMemPtr, indexBuffer.data(), (std::size_t)indexBufferSize);
		vkUnmapMemory(this->device_.logicalDevice, primitive.indexBuffer.vkDeviceMemory);
	}

		// �������� ����� �������� � ������
		this->primitives_.push_back(primitive);

		// �������� ��������� �����
		this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
		this->PrepareDrawCommands(this->commandBuffersDraw_, this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_, this->swapchain_, this->primitives_);

		// ������� ������
		return (unsigned int)(this->primitives_.size() - 1);
	
	
}

//���������� ���������
void MainRenderer::UpdatePrimitive(unsigned int primitieInc, glm::vec3 position, glm::vec3 rotaton, glm::vec3 scale)
{
	VKStr::Primitive primitive;
	primitive = this->primitives_[primitieInc];
	primitive.position = position;
	primitive.rotation = rotaton;
	primitive.scale = scale;
	
	// �������� ����� �������� � ������
	this->primitives_[primitieInc]=primitive;

	// �������� ��������� �����
	this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);
	this->PrepareDrawCommands(this->commandBuffersDraw_, 
		this->renderPass_, this->pipelineLayout_, this->descriptorSetMain_, this->descriptorSetInfo_, this->descriptorSetVLight_,
		this->swapchain_, this->primitives_);
}

//������������� ����� (������, ������� ����� ������������ � ������ ����������, ��� ����������������� ��������
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
		this->uboVectorsLight_[fsInfo_.LCount].outvec4 = glm::vec4(constant, linear, quadratic, ambientRate);//��������� - �������� �� 4 ����. ���� ����� ��������� 1 ��������� ���� float


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
	// ������������� ���������� �������� ������ (���� �����-���� � ��������)
	this->Pause();

	
	VKStr::Texture resultTexture = {};

	// ������ ����������� (������� �� ��������� 4 ����� �� �������, � ������ RGBA)
	VkDeviceSize size = (VkDeviceSize)(width * height * bpp);

	// ���� ������ �� ����������
	if (!pixels) {
		throw std::runtime_error("Vulkan: Error while creating texture. Empty pixel buffer recieved (createTexture)");
	}

	// ������� ������������� �����������
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

	// ������� ��������� ����������� (���-������� 0, ���� - 0)
	VkImageSubresource subresource = {};
	subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource.mipLevel = 0;
	subresource.arrayLayer = 0;

	// ���������� ���� � ����������
	VkSubresourceLayout stagingImageSubresourceLayout = {};
	vkGetImageSubresourceLayout(this->device_.logicalDevice, stagingImage.vkImage, &subresource, &stagingImageSubresourceLayout);

	// ��������� ������ ��� �����������
	void* data;
	vkMapMemory(this->device_.logicalDevice, stagingImage.vkDeviceMemory, 0, size, 0, &data);

	// ���� "������ ������" ����� ���-�� �������� �� ������ ������������ �� bpp - ����� ����������� ������� memcpy
	if (stagingImageSubresourceLayout.rowPitch == width * bpp) {
		memcpy(data, pixels, (unsigned int)size);
	}
	// ���� ��� (�������� ������ ����������� �� ������ ������� ������) - ���������� ����� �� ��������� � �������� ������ �������
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

	// ������ �������� ������
	vkUnmapMemory(this->device_.logicalDevice, stagingImage.vkDeviceMemory);

	// ������� ��������� ����������� (� ������ ����������)
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


	// ������� ��������� ����� ��� ������ �������� ���������� �����������
	VkCommandBuffer transitionCmdBuffer = CreateSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_);

	// ��������� �������������� ����� ���������� � ������������ (��������� ���)
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	// ������� ���������� ������ �������������� ����������� � VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
	CmdImageLayoutTransition(transitionCmdBuffer, stagingImage.vkImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

	// ������� ���������� ������ �������� ����������� � VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	CmdImageLayoutTransition(transitionCmdBuffer, resultTexture.image.vkImage, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

	// ��������� ������� �������� ����������
	FlushSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_, transitionCmdBuffer, this->device_.queues.graphics);

	// ������� ��������� ����� ��� ����������� �����������
	VkCommandBuffer copyCmdBuffer = CreateSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_);

	// ����������� �� ������������� �������� � ��������
	CmdImageCopy(copyCmdBuffer, stagingImage.vkImage, resultTexture.image.vkImage, width, height);

	// ��������� ������� �����������
	FlushSingleTimeCommandBuffer(this->device_, this->commandPoolDraw_, copyCmdBuffer, this->device_.queues.graphics);

	// �������� ������������� �����������
	stagingImage.Deinit(this->device_.logicalDevice);


	// �������� ����� ����� ������������ �� ��������������� ����
	VkDescriptorSetAllocateInfo descriptorSetAllocInfo = {};
	descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocInfo.descriptorPool = this->descriptorPoolTextures_;
	descriptorSetAllocInfo.descriptorSetCount = 1;
	descriptorSetAllocInfo.pSetLayouts = &(this->descriptorSetLayoutTextures_);

	if (vkAllocateDescriptorSets(this->device_.logicalDevice, &descriptorSetAllocInfo, &(resultTexture.descriptorSet)) != VK_SUCCESS) {
		throw std::runtime_error("Vulkan: Error in vkAllocateDescriptorSets. Can't allocate descriptor set for texture (createTexture)");
	}

	// ���������� � ������������ �����������
	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = resultTexture.image.vkImageView;
	imageInfo.sampler = this->textureSampler_;

	// ������������ ����������� � ����� ������������
	std::vector<VkWriteDescriptorSet> writes =
	{
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,      // ��� ���������
			nullptr,                                     // pNext
			resultTexture.descriptorSet,                 // ������� ����� ������������
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

	// ���������� ������� ������ ����� ��������
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
	// ��������� ��������� ������ �� ���������
	this->camera_.fFar = DEFAULT_FOV;
	this->camera_.fFar = DEFAULT_FAR;
	this->camera_.fNear = DEFAULT_NEAR;

	// ������ ���������� � ����� ������������� �� ���������
	std::vector<const char*> instanceExtensionsRequired = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
	std::vector<const char*> deviceExtensionsRequired = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	std::vector<const char*> validationLayersRequired = {};

	// ���� ��� DEBUG ������������ - ��������� ��� ���������� � ���� ��� ���������
	if (IS_VK_DEBUG) {
		instanceExtensionsRequired.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		validationLayersRequired.push_back("VK_LAYER_LUNARG_standard_validation");
	}

	// ������������� ����������
	this->instance_ = this->initInstance("vulkan is trash", "and this engine is trash", instanceExtensionsRequired, validationLayersRequired);

	// ������������� ����������� �����������
	this->surface_ = this->initWindowSurface(this->instance_, hInstance, hWnd);

	// ������������� ����������
	this->device_ = this->initDevice(this->instance_, this->surface_, deviceExtensionsRequired, validationLayersRequired, false);
	// ������������� ������� ����������
	this->renderPass_ = this->initRenderPassage(this->device_, this->surface_, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_D32_SFLOAT_S8_UINT);
	this->shadowPass_.renderPass = this->initShadowRenderPassage(this->device_);
	this->initShadowFrameBuffer(this->device_);

	// ������������� swap-chain
	this->swapchain_ = this->initSwapChain(this->device_, this->surface_, { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }, VK_FORMAT_D32_SFLOAT_S8_UINT, this->renderPass_, 3, nullptr);

	// ������������� ���������� ����
	this->commandPoolDraw_ = this->initCommandPool(this->device_, this->device_.queueFamilies.graphics);

	// ��������� ��������� ������� (��������� �������)
	this->commandBuffersDraw_ = this->initCommandBuffers(this->device_, this->commandPoolDraw_, (unsigned int)(this->swapchain_.framebuffers.size()));

	// ��������� ����������� uniform-������
	this->uniformBufferWorld_ = this->initStandartUnformBuffer(this->device_, 
																sizeof(VKStr::UboWorld),
																VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// ��������� uniform-������ ��������� �������� (������������ �����)
	this->uniformBufferModels_ = this->initStandartUnformBuffer(this->device_, 
																this->device_.GetDynamicAlignment<glm::mat4>() * this->primitivesMaxCount_,
																VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	// �������� �������������� ���� ��� ��������� ��������� ������ (��� unform-������)
	this->descriptorPoolMain_ = this->initDescriptorPoolMain(this->device_);

	// �������� �������������� ���� ��� ��������� ����������� ������ (���������� ��������)
	this->descriptorPoolTextures_ = this->initDescriptorPoolTextures(this->device_);

	// ������������� ���������� ��������� �������������� ������
	this->descriptorSetLayoutMain_ = this->initDescriptorSetLayoutMain(this->device_);

	// ������������� ���������� ������������ ������
	this->descriptorSetLayoutTextures_ = this->initDescriptorSetLayoutTextures(this->device_);

	

	// ������������� ����������� ��������
	this->textureSampler_ = this->initTextureSampler(this->device_);

	// ������������� �������������� ������
	this->descriptorSetMain_ = this->initDescriptorSetMain(
		this->device_,
		this->descriptorPoolMain_,
		this->descriptorSetLayoutMain_,
		this->uniformBufferWorld_,
		this->uniformBufferModels_);

	//��� ���������� �����
	this->uniformBufferInfo_ = this->initStandartUnformBuffer(this->device_,
																sizeof(VKStr::FSInfo),
																VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	this->uniformBufferVectorsLight_ = this->initStandartUnformBuffer(this->device_, 
																	sizeof(VKStr::UBOVectorsLight)*128,
																	VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
																	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	//��� ���������� ������� �����
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

	//��� ������ � ���������� �������
	this->descriptorPoolShadowSempler_ = this->initDescriptorPoolTextures(this->device_);
	this->descriptorSetLayoutShadowSempler_ = this->initDescriptorSetLayoutTextures(this->device_);
	this->descriptorSetShadowSempler_ = this->initShadowSamplerDescriptorSet(this->device_, this->descriptorPoolShadowSempler_, this->descriptorSetLayoutShadowSempler_, this->shadowPass_);

	// ��������� ������ ������� ubo-�������� ��������� ����������
	this->uboModels_ = this->AllocateUboModels(this->device_, this->primitivesMaxCount_);

	// ������������� ���������� ������������ ���������
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
		// ������������� ������������ ���������
		this->pipelines_.push_back(this->initGraphicsPipeline(this->device_, this->pipelineLayout_[0], this->swapchain_, this->renderPass_, "base.frag.spv", "base.vert.spv"));
		this->pipelines_.push_back(this->initSBGraphicsPipeline(this->device_, this->pipelineLayout_[0], this->swapchain_, this->renderPass_, "sb.frag.spv", "sb.vert.spv"));
		this->ShadowRender = true;
	}
	this->pipelines_.push_back( this->initGraphicsPipeline(this->device_, this->pipelineLayout_[1], this->swapchain_, this->renderPass_, "baseTestLight.frag.spv", "baseTestLight.vert.spv"));
	this->pipelines_.push_back(this->initShadowPipeline(this->device_, this->pipelineLayout_[2], this->shadowPass_.renderPass, "ShadowMap.vert.spv"));


	// ��������� �������������
	this->sync_ = this->InitSynchronization(this->device_);

	// ���������� ������� �������
	this->PrepareDrawCommands(
		this->commandBuffersDraw_,
		this->renderPass_,
		this->pipelineLayout_,
		this->descriptorSetMain_,
		this->descriptorSetInfo_,
		this->descriptorSetVLight_,
		this->swapchain_,
		this->primitives_);


	// ������ � ����������
	this->isReady_ = true;

	// ��������
	this->Update();
	
}

	MainRenderer::~MainRenderer()
	{
		this->Pause();
		this->isReady_ = false;

		// ����� ������� ������
		this->ResetCommandBuffers(this->device_, this->commandBuffersDraw_);

		// ��������������� ���������� �������������
		this->DeinitSynchronization(this->device_, &(this->sync_));

		// ��������������� ������������ ���������
		for (size_t i = 0; i < this->pipelines_.size(); i++)
		{
			this->DeinitGraphicsPipeline(this->device_, &(this->pipelines_[i]));
			pipelines_.pop_back();
		}

		// ��������������� ������������ ���������
		for (size_t i = 0; i<this->pipelineLayout_.size(); i++)
			this->DeinitPipelineLayout(this->device_, &(this->pipelineLayout_[i]));

		// ������� ������ ������� ubo-�������� ��������� ����������
		this->FreeUboModels(&(this->uboModels_));

		// ��������������� ������ ������������
		this->DeinitDescriptorSet(this->device_, this->descriptorPoolMain_, &(this->descriptorSetMain_));

		// ��������������� ����������� ��������
		this->DeinitTextureSampler(this->device_, &(this->textureSampler_));

		this->DeinitDescriptorSet(this->device_, this->descriptorPoolInfo_, &(this->descriptorSetInfo_));


		this->DeinitDescriptorSet(this->device_, this->descriptorPoolInfo_, &(this->descriptorSetVLight_));

		this->DeinitDescriptorSet(this->device_, this->descriptorPoolInfo_, &(this->descriptorSetShadow_));

		// ��������������� ���������� ����������� �������������� ������
		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutTextures_));

		// ��������������� ���������� ���������� �������������� ������
		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutMain_));


		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutVLight_));
		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutShadowSempler_));

		this->DeinitDescriporSetLayout(this->device_, &(this->descriptorSetLayoutShadow_));

		// ����������� ntrcnehyjuj �������������� ����
		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolTextures_));

		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolShadowSempler_));

		// ����������� ��������� �������������� ����
		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolMain_));

		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolInfo_));


		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolVLight_));

		this->DeinitDescriptorPool(this->device_, &(this->descriptorPoolShadow_));

		// ��������������� uniform-������� ��������
		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferModels_));

		// ��������������� ����������� uniform-�������
		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferWorld_));

		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferVectorsLight_));


		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferInfo_));

		this->DeinitUniformBuffer(this->device_, &(this->uniformBufferVectorsLight_));

		this->DeinitUniformBuffer(this->device_, &(this->ShadowBuff_));

		// ��������������� ��������� �������
		this->DeinitCommandBuffers(this->device_, this->commandPoolDraw_, &(this->commandBuffersDraw_));

		// ��������������� ���������� ����
		this->DeinitCommandPool(this->device_, &(this->commandPoolDraw_));

		// ��������������� swap-chain'�
		this->DeinitSwapchain(this->device_, &(this->swapchain_));

		// �������������� ������� ����������
		this->DeinitRenderPassage(this->device_, &(this->renderPass_));
		this->DeinitRenderPassage(this->device_, &(this->shadowPass_.renderPass));

		// �������������� ����������
		this->DeinitDevice(&(this->device_));

		// ��������������� �����������
		this->DeinitWindowSurface(this->instance_, &(this->surface_));

		// ��������������� ���������� Vulkan
		this->DeinitInstance(&(this->instance_));
	}


