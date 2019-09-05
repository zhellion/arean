
#pragma once



#ifndef VKRENDERER_H
#define VKRENDERER_H




#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>

// ��������� ���������� ���������� � �� (Window System Integration)
// ���� ������ ��� �������� � Windows
#define VK_USE_PLATFORM_WIN32_KHR

// ��������� ������ �� ��������� (���� ������, ������� ���������)
#define DEFAULT_FOV 60.0f
#define DEFAULT_NEAR 0.1f
#define DEFAULT_FAR 256.0f

// ������� ��������
#define AREAN_SHADER_T_BASE (int)0
#define AREAN_SHADER_T_TEST_LIGHT (int)2
#define AREAN_SHADER_T_SKY_BOX (int)1


//
#define NULL_GLM_3 {}

// �������� �������� ������� � OpenGL �� -1 �� 1. � Vulkan - �� 0 �� 1 (��� � DirectX)
// ������ ������ "�������" GLM ��� ����� ������������ �������� �� 0 �� 1, ��� ��������
// �� ���������� ������ ��������, ������� ������������ � �������
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// ���������� vulkan
#include <vulkan\vulkan.h>

// ���������� glm ��� ������ � ���������
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

// ��������������� �����������
#include "tools.h"
#include "VKStr.h"



#ifdef NDEBUG
const bool IS_VK_DEBUG = false;
#else
const bool IS_VK_DEBUG = true;
#endif

class MainRenderer
{
private:
	bool isReady_;                                        // ��������� ���������� � ����������
	bool isRendering_;                                    // � �������� �� ���������

	VkInstance instance_;                                 // ����� instance'� vulkan'�
	VkDebugReportCallbackEXT validationReportCallback_;   // ����� ������� callback'�
	VkSurfaceKHR surface_;                                // ����� ����������� �����������
	VKStr::Device device_;                            // ���������� (��������� � �������� ���-�� � ���-�� ��-��, ��������)
	VkRenderPass renderPass_;                             // �������� ������ ����������
	VKStr::Swapchain swapchain_;                      // ����-���� (����� ������, ����� ����������� ������� �����)
	VkCommandPool commandPoolDraw_;                       // ��������� ��� (��� ��������� ��������� �������)
	std::vector<VkCommandBuffer> commandBuffersDraw_;     // ��������� ������ (���� �� ������ �����. swap-chain, � ������� ������ ��� � � ������, � ����� �������������)

	VKStr::UniformBuffer uniformBufferWorld_;         // ����� ����� ����� (�������� ������ ������, ������, ���� ��� �����������)
	VKStr::UniformBuffer uniformBufferModels_;        // ����� ����� �������� (�������� ������ ������, ������, ���� ��� �����������)
	VKStr::UboWorld uboWorld_;                        // ��������� � ��������� ��� ����� �������������� ����� (������ ������ ����� ������������ � ����� ����� �����)
	VKStr::UboModelArray uboModels_;                  // ������ ������ (��������� �� ����) ��� ��������� �������� (������� ������, ���������� � ����� ����� ��������)
	VKStr::UBOVectorsLight uboVectorsLight_[128];
	VKStr::UniformBuffer ShadowBuff_; //������ ��� ���������� ������� �����
	VKStr::UboModelArray ShadowMat4_;
	bool ShadowRender = false;
	VKStr::ShadowPass shadowPass_;

	VkDescriptorPool descriptorPoolShadowSempler_;             
	VkDescriptorSetLayout descriptorSetLayoutShadowSempler_;
	VkDescriptorSet descriptorSetShadowSempler_;



	glm::vec3 SelectedObject = glm::vec3(0.0f, 0.0f, 0.0f); //���������� ������ (���� �� ��������, ��������� �� ����� �����)

	VKStr::UniformBuffer uniformBufferVectorsLight_;

	VKStr::UniformBuffer uniformBufferInfo_;


	VkDescriptorPool descriptorPoolMain_;                 // ��� ������������ (��� ��������� ������)
	VkDescriptorSetLayout descriptorSetLayoutMain_;       // ���������� ������ ������������ (��� ��������� ������)
	VkDescriptorSet descriptorSetMain_;                   // ����� ������������ (��������)

	VkDescriptorPool descriptorPoolTextures_;             // ��� ������������ (��� ������� ��� ��������)
	VkDescriptorSetLayout descriptorSetLayoutTextures_;   // ���������� ������ ������������ (��� ������� ��� ��������)
	VkSampler textureSampler_;                            // ���������� ������� (��������� ��� ������ �������� � ������ � ��� ���������������� ����������)

	VkDescriptorPool descriptorPoolInfo_;             
	VkDescriptorSetLayout descriptorSetLayoutInfo_;   
	VkDescriptorSet descriptorSetInfo_;


	VkDescriptorPool descriptorPoolVLight_;
	VkDescriptorSetLayout descriptorSetLayoutVLight_;
	VkDescriptorSet descriptorSetVLight_;

	VkDescriptorPool descriptorPoolShadow_;
	VkDescriptorSetLayout descriptorSetLayoutShadow_;
	VkDescriptorSet descriptorSetShadow_;

	std::vector<VkPipelineLayout> pipelineLayout_;                     // ���������� ���������
	std::vector<VkPipeline> pipelines_;                                 // �������� ����������� ��������
	VKStr::Synchronization sync_;                     // ��������� �������������

	unsigned int primitivesMaxCount_;                     // ������������ ���-�� ���������� (���������� ��� ��������� ������������� UBO ������)
	std::vector<VKStr::Primitive> primitives_;        // ����� �������. ���������� ��� �����������
	VKStr::FSInfo fsInfo_;




	VKStr::CameraSettings camera_;                    // ��������� ������
	glm::vec3 outCamPos;


	// INSTANCE //����� ������� �����
	VkInstance initInstance(
		std::string applicationName,
		std::string engineName,
		std::vector<const char*> extensionsRequired,
		std::vector<const char*> validationLayersRequired);
	void DeinitInstance(VkInstance * vkInstance);
	bool CheckInstanceExtensionsSupported(std::vector<const char*> instanceExtensionsNames);

	//Validatuion Layers //���������� ������
	bool CheckValidationsLayersSupported(std::vector<const char*> validationLayersNames);
	
	//SURFACE //�����������, �� ���� ����� ��� ��������
	VkSurfaceKHR initWindowSurface(VkInstance vkInstance, HINSTANCE hInstance, HWND hWnd);
	void DeinitWindowSurface(VkInstance vkInstance, VkSurfaceKHR * surface);
	VKStr::SurfaceInfo GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	
	

	//Device //��������� ����������� ���������� � ����� � ���������� ����������
	VKStr::Device initDevice(VkInstance vkInstance, VkSurfaceKHR surface, std::vector<const char*> extensionsRequired,
		std::vector<const char*> validationLayersRequired, bool uniqueQueueFamilies);

	VKStr::QueueFamilyInfo GetQueueFamilyInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool uniqueStrict = false);

	int GetMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int typeFlags, VkMemoryPropertyFlags properties); //�������� ��� ������ � ����������� �������������

	bool CheckDeviceExtensionSupported(VkPhysicalDevice physicalDevice, std::vector<const char*> deviceExtensionsNames);//��������� ��������� ����������

	void DeinitDevice(VKStr::Device * device);

	//Render passage //������ ����������, ��������� ������ ������ ����������
	VkRenderPass initRenderPassage(const VKStr::Device &device, VkSurfaceKHR surface, VkFormat colorAttachmentFormat, VkFormat depthStencilFormat);

	VkRenderPass initShadowRenderPassage(const VKStr::Device &device);

	void DeinitRenderPassage(const VKStr::Device &device, VkRenderPass * renderPass);

	//swapchain //��������� ����� ������

	
	VKStr::Swapchain initSwapChain(const VKStr::Device &device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkFormat depthStencilFormat,
		VkRenderPass renderPass, unsigned int bufferCount, VKStr::Swapchain * oldSwapchain = nullptr);

	VKStr::Image CreateImageSingle(const VKStr::Device &device, VkImageType imageType, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage,
		VkImageAspectFlags subresourceRangeAspect, VkImageLayout initialLayout, VkMemoryPropertyFlags memoryProperties, VkImageTiling tiling,
		VkSampleCountFlagBits samples, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);//�������� ������������ �����������

	void initShadowFrameBuffer(const VKStr::Device &device);


	void DeinitSwapchain(const VKStr::Device &device, VKStr::Swapchain * swapchain);

	//Command pools and buffers //��������� ��������� ����� � �������, ����� ��� ���������� �����������. ��� �� ���� ������� ������������� � ����������� ������.
	VkCommandPool initCommandPool(const VKStr::Device &device, unsigned int queueFamilyIndex);

	
	void DeinitCommandPool(const VKStr::Device &device, VkCommandPool * commandPool);


	std::vector<VkCommandBuffer> initCommandBuffers(const VKStr::Device &device, VkCommandPool commandPool, unsigned int count);

	void DeinitCommandBuffers(const VKStr::Device &device, VkCommandPool commandPool, std::vector<VkCommandBuffer> * buffers);

	VKStr::Synchronization InitSynchronization(const VKStr::Device &device);

	
	void DeinitSynchronization(const VKStr::Device &device, VKStr::Synchronization * sync);

	void PrepareDrawCommands(std::vector<VkCommandBuffer> commandBuffers, VkRenderPass renderPass, std::vector<VkPipelineLayout> pipelineLayout, VkDescriptorSet descriptorSetMain, VkDescriptorSet descriptorSetInfo, VkDescriptorSet descriptorSetVLight, const VKStr::Swapchain & swapchain, const std::vector<VKStr::Primitive>& primitives);


	void ResetCommandBuffers(const VKStr::Device &device, std::vector<VkCommandBuffer> commandBuffers);

	VkCommandBuffer CreateSingleTimeCommandBuffer(const VKStr::Device &device, VkCommandPool commandPool);

	void FlushSingleTimeCommandBuffer(const VKStr::Device &device, VkCommandPool commandPool, VkCommandBuffer commandBuffer, VkQueue queue);

	//uniform buffer //��������� ����� ����������� ������ �������

	
	VKStr::Buffer CreateBuffer(const VKStr::Device &device, VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);


	VKStr::UniformBuffer initStandartUnformBuffer(const VKStr::Device &device, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	

	
	void DeinitUniformBuffer(const VKStr::Device &device, VKStr::UniformBuffer * uniformBuffer);

	//descriptor and sempler //��������� ����� ��� �������� ������ � ������

	
	VkDescriptorPool initDescriptorPoolMain(const VKStr::Device &device);


	VkDescriptorPool initDescriptorPoolTextures(const VKStr::Device &device, uint32_t maxDescriptorSets = 1000);
	VkDescriptorPool initDescriptorPoolLight(const VKStr::Device &device);

	
	void DeinitDescriptorPool(const VKStr::Device &device, VkDescriptorPool * descriptorPool);

	
	VkDescriptorSetLayout initDescriptorSetLayoutMain(const VKStr::Device &device);

	
	VkDescriptorSetLayout initDescriptorSetLayoutTextures(const VKStr::Device &device);

	VkDescriptorSetLayout initDescriptorSetLayoutLight(const VKStr::Device & device);

	
	void DeinitDescriporSetLayout(const VKStr::Device &device, VkDescriptorSetLayout * descriptorSetLayout);


	VkSampler initTextureSampler(const VKStr::Device &device);

	
	void DeinitTextureSampler(const VKStr::Device &device, VkSampler * sampler);

	
	VkDescriptorSet initDescriptorSetMain(const VKStr::Device &device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
		const VKStr::UniformBuffer &uniformBufferWorld, const VKStr::UniformBuffer &uniformBufferModels);

	VkDescriptorSet initDescriptorSetLight(const VKStr::Device & device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
		const VKStr::UniformBuffer & uniformBufferLCount);

	VkDescriptorSetLayout initShadowDescriptorSetLayoutMain(const VKStr::Device & device);
	//------------------------------------------------------------

	VkDescriptorSet MainRenderer::initShadowSamplerDescriptorSet(const VKStr::Device & device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
		VKStr::ShadowPass shadowPass);
	//------------------------------------------------------------


	VkDescriptorSet initDescriptorSetVLight(const VKStr::Device & device, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout,
		const VKStr::UniformBuffer & uniformBufferVectorsLight);
	VkDescriptorSetLayout initDescriptorSetLayoutVLight(const VKStr::Device & device);
	VkDescriptorPool initDescriptorPoolVLight(const VKStr::Device &device);

	//-----------------------------------------------------------

	
	void DeinitDescriptorSet(const VKStr::Device &device, VkDescriptorPool descriptorPool, VkDescriptorSet * descriptorSet);

	
	VKStr::UboModelArray AllocateUboModels(const VKStr::Device &device, unsigned int maxObjects);

	
	void FreeUboModels(VKStr::UboModelArray * uboModels);


	//Pipelines (��������)  //��� ������������ �������, ��� �� ��������� ����������� ���������

	VkPipelineLayout initPipelineLayout(const VKStr::Device &device, std::vector<VkDescriptorSetLayout> descriptorSetLayouts);

	void DeinitPipelineLayout(const VKStr::Device &device, VkPipelineLayout * pipelineLayout);

	std::vector<VkVertexInputBindingDescription> GetVertexInputBindingDescriptions(unsigned int bindingIndex);
	std::vector<VkVertexInputAttributeDescription> GetVertexInputAttributeDescriptions(unsigned int bindingIndex);

	VkShaderModule LoadSPIRVShader(std::string filename, VkDevice logicalDevice);

	VkPipeline initGraphicsPipeline(const VKStr::Device &device, VkPipelineLayout pipelineLayout, const VKStr::Swapchain &swapchain, VkRenderPass renderPass, std::string fShaderName, std::string vShaderName);
	VkPipeline initSBGraphicsPipeline(const VKStr::Device &device, VkPipelineLayout pipelineLayout, const VKStr::Swapchain &swapchain, VkRenderPass renderPass, std::string fShaderName, std::string vShaderName);

	void DeinitGraphicsPipeline(const VKStr::Device &device, VkPipeline * pipeline);

	//shadowmap
	VkPipeline initShadowPipeline(const VKStr::Device & device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, std::string vShaderName);

	
	//image ������ � ����������� 
	void CmdImageLayoutTransition(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);

	void CmdDepthLayoutTransition(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);

	void CmdImageCopy(VkCommandBuffer cmdBuffer, VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
	

public:

	/**
	* ������������ ���������� � ��������� ���������� ���������� ���� ������
	*/
	void Pause();

	/**
	* ������� � ��������� "����������"
	*/
	void Continue();

	/**
	* ������ ����� ���������� ��� ����� ���������� ����������� �����������
	* ���� (� ����������) ��� ����� �����-���� ���� �������� �������. � ��� ����������
	* ������������ swap-chain'� � ����� ����, ��� ������� �� ���������� ��������
	*/
	void VideoSettingsChanged();

	/**
	* � ������ ��������� ���������� �������� �������������� ������ � ���-�� �����
	* ������� ����������� �� ����������� ������
	*/
	void Draw();

	/**
	* � ������ ���������� ���������� �������� ����� ������ � UBO ������, �� ����
	* ����������� ��������� ������, ��������� ���������� � ����� � �����
	*/
	void Update();

	// ��������� ���������� ����������� ������ (���� ������, ����� ���������)
	
	void SetCameraPerspectiveSettings(float fFOV, float fNear, float fFar);

	// ��������� ��������� ������
	
	void SetCameraPosition(float x, float y, float z);

	// ��������� �������� ������
	
	void SetCameraRotation(float x, float y, float z);
	glm::vec3 CameraPos();

	// ���������� ������ ���������
	
	unsigned int AddPrimitive(const std::vector<VKStr::Vertex> &vertices,const std::vector<unsigned int> &indices, 
		const VKStr::Texture *texture, int pipelineIndex, glm::vec3 position, glm::vec3 rotaton, glm::vec3 scale = { 1.0f,1.0f,1.0f });
	void UpdatePrimitive(unsigned int primitieInc, glm::vec3 position, glm::vec3 rotaton, glm::vec3 scale = { 1.0f,1.0f,1.0f });
	//����
	unsigned int initLight(glm::vec3 position, glm::vec3 LightColor, float ambientRate, float diffuseRate, float specularRate, float constant = 1.0f, float linear = 0.007f, float quadratic = 0.0002f);
	void reInitLight(glm::vec3 position, int IDLight);
	void DeleteLight(int IDLight);
	// �������� �������� 
	VKStr::Texture CreateTexture(const unsigned char* pixels, uint32_t width, uint32_t height, uint32_t channels, uint32_t bpp = 4);
	

	// ����������� ���������
	MainRenderer(HINSTANCE hInstance, HWND hWnd, unsigned int primitivesMaxCount = 100);

	
	~MainRenderer();
};



#endif // !1