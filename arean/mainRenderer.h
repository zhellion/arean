
#pragma once



#ifndef VKRENDERER_H
#define VKRENDERER_H




#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>

// Активация расширения интеграции с ОС (Window System Integration)
// Даем понять что работаем с Windows
#define VK_USE_PLATFORM_WIN32_KHR

// Параметры камеры по умолчанию (угол обзора, границы отсечения)
#define DEFAULT_FOV 60.0f
#define DEFAULT_NEAR 0.1f
#define DEFAULT_FAR 256.0f

// индексы шейдеров
#define AREAN_SHADER_T_BASE (int)0
#define AREAN_SHADER_T_TEST_LIGHT (int)2
#define AREAN_SHADER_T_SKY_BOX (int)1


//
#define NULL_GLM_3 {}

// Интервал значений глубины в OpenGL от -1 до 1. В Vulkan - от 0 до 1 (как в DirectX)
// Данный символ "сообщит" GLM что нужно использовать интервал от 0 до 1, что скажется
// на построении матриц проекции, которые используются в шейдере
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// Подключаем vulkan
#include <vulkan\vulkan.h>

// Подключаем glm для работы с матрицами
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

// Вспомогательные инструменты
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
	bool isReady_;                                        // Состояние готовности к рендерингу
	bool isRendering_;                                    // В процессе ли рендеринг

	VkInstance instance_;                                 // Хендл instance'а vulkan'а
	VkDebugReportCallbackEXT validationReportCallback_;   // Хендл объекта callback'а
	VkSurfaceKHR surface_;                                // Хендл поверхности отображения
	VKStr::Device device_;                            // Устройство (структура с хендлами физ-го и лог-го ус-ва, очередей)
	VkRenderPass renderPass_;                             // Основной проход рендеринга
	VKStr::Swapchain swapchain_;                      // Своп-чейн (набор показа, набор сменяющихся буферов кадра)
	VkCommandPool commandPoolDraw_;                       // Командный пул (для выделения командных буферов)
	std::vector<VkCommandBuffer> commandBuffersDraw_;     // Командные буферы (свой на каждое изобр. swap-chain, с набором команд что и в других, в целях синхронизации)

	VKStr::UniformBuffer uniformBufferWorld_;         // Буфер формы сцены (содержит хендлы буфера, памяти, инфо для дескриптора)
	VKStr::UniformBuffer uniformBufferModels_;        // Буфер формы объектов (содержит хендлы буфера, памяти, инфо для дескриптора)
	VKStr::UboWorld uboWorld_;                        // Структура с матрицами для общих преобразований сцены (данный объект буедт передаваться в буфер формы сцены)
	VKStr::UboModelArray uboModels_;                  // Массив матриц (указатель на него) для отдельный объектов (матрицы модели, передаются в буфер формы объектов)
	VKStr::UBOVectorsLight uboVectorsLight_[128];
	VKStr::UniformBuffer ShadowBuff_; //буферы для подпрохода рендера теней
	VKStr::UboModelArray ShadowMat4_;
	bool ShadowRender = false;
	VKStr::ShadowPass shadowPass_;

	VkDescriptorPool descriptorPoolShadowSempler_;             
	VkDescriptorSetLayout descriptorSetLayoutShadowSempler_;
	VkDescriptorSet descriptorSetShadowSempler_;



	glm::vec3 SelectedObject = glm::vec3(0.0f, 0.0f, 0.0f); //веделенный объект (пока не доступно, указывает на центр сцены)

	VKStr::UniformBuffer uniformBufferVectorsLight_;

	VKStr::UniformBuffer uniformBufferInfo_;


	VkDescriptorPool descriptorPoolMain_;                 // Пул дескрипторов (для основного набора)
	VkDescriptorSetLayout descriptorSetLayoutMain_;       // Размещение набора дескрипторов (для основного набора)
	VkDescriptorSet descriptorSetMain_;                   // Набор дескрипторов (основной)

	VkDescriptorPool descriptorPoolTextures_;             // Пул дескрипторов (для наборов под текстуры)
	VkDescriptorSetLayout descriptorSetLayoutTextures_;   // Размещение набора дескрипторов (для наборов под текстуры)
	VkSampler textureSampler_;                            // Текстурный семплер (описывает как данные подаются в шейдер и как интерпретируются координаты)

	VkDescriptorPool descriptorPoolInfo_;             
	VkDescriptorSetLayout descriptorSetLayoutInfo_;   
	VkDescriptorSet descriptorSetInfo_;


	VkDescriptorPool descriptorPoolVLight_;
	VkDescriptorSetLayout descriptorSetLayoutVLight_;
	VkDescriptorSet descriptorSetVLight_;

	VkDescriptorPool descriptorPoolShadow_;
	VkDescriptorSetLayout descriptorSetLayoutShadow_;
	VkDescriptorSet descriptorSetShadow_;

	std::vector<VkPipelineLayout> pipelineLayout_;                     // Размещение конвейера
	std::vector<VkPipeline> pipelines_;                                 // Основной графический конвейер
	VKStr::Synchronization sync_;                     // Примитивы синхронизации

	unsigned int primitivesMaxCount_;                     // Максимальное кол-во примитивов (необходимо для аллокации динамического UBO буфера)
	std::vector<VKStr::Primitive> primitives_;        // Набор геометр. примитивов для отображения
	VKStr::FSInfo fsInfo_;




	VKStr::CameraSettings camera_;                    // Параметры камеры
	glm::vec3 outCamPos;


	// INSTANCE //самая главная фигня
	VkInstance initInstance(
		std::string applicationName,
		std::string engineName,
		std::vector<const char*> extensionsRequired,
		std::vector<const char*> validationLayersRequired);
	void DeinitInstance(VkInstance * vkInstance);
	bool CheckInstanceExtensionsSupported(std::vector<const char*> instanceExtensionsNames);

	//Validatuion Layers //обработчик ошибок
	bool CheckValidationsLayersSupported(std::vector<const char*> validationLayersNames);
	
	//SURFACE //поверхность, то куда будет все выводить
	VkSurfaceKHR initWindowSurface(VkInstance vkInstance, HINSTANCE hInstance, HWND hWnd);
	void DeinitWindowSurface(VkInstance vkInstance, VkSurfaceKHR * surface);
	VKStr::SurfaceInfo GetSurfaceInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	
	

	//Device //настройка логического устройства и связи с физическим устойством
	VKStr::Device initDevice(VkInstance vkInstance, VkSurfaceKHR surface, std::vector<const char*> extensionsRequired,
		std::vector<const char*> validationLayersRequired, bool uniqueQueueFamilies);

	VKStr::QueueFamilyInfo GetQueueFamilyInfo(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool uniqueStrict = false);

	int GetMemoryTypeIndex(VkPhysicalDevice physicalDevice, unsigned int typeFlags, VkMemoryPropertyFlags properties); //Получить тип памяти с конкретными особенностями

	bool CheckDeviceExtensionSupported(VkPhysicalDevice physicalDevice, std::vector<const char*> deviceExtensionsNames);//проверяем поддержку расширений

	void DeinitDevice(VKStr::Device * device);

	//Render passage //проход рендеринга, настройка этапов самого ренжеринга
	VkRenderPass initRenderPassage(const VKStr::Device &device, VkSurfaceKHR surface, VkFormat colorAttachmentFormat, VkFormat depthStencilFormat);

	VkRenderPass initShadowRenderPassage(const VKStr::Device &device);

	void DeinitRenderPassage(const VKStr::Device &device, VkRenderPass * renderPass);

	//swapchain //настройка смены кадров

	
	VKStr::Swapchain initSwapChain(const VKStr::Device &device, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat, VkFormat depthStencilFormat,
		VkRenderPass renderPass, unsigned int bufferCount, VKStr::Swapchain * oldSwapchain = nullptr);

	VKStr::Image CreateImageSingle(const VKStr::Device &device, VkImageType imageType, VkFormat format, VkExtent3D extent, VkImageUsageFlags usage,
		VkImageAspectFlags subresourceRangeAspect, VkImageLayout initialLayout, VkMemoryPropertyFlags memoryProperties, VkImageTiling tiling,
		VkSampleCountFlagBits samples, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);//создание однослойного изображения

	void initShadowFrameBuffer(const VKStr::Device &device);


	void DeinitSwapchain(const VKStr::Device &device, VKStr::Swapchain * swapchain);

	//Command pools and buffers //настройка командных пулов и буферов, нужны для управления устройством. Так же сюда отнесем синхронизацию с подготовкой команд.
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

	//uniform buffer //настройка связи глобального буфера шейдера

	
	VKStr::Buffer CreateBuffer(const VKStr::Device &device, VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE);


	VKStr::UniformBuffer initStandartUnformBuffer(const VKStr::Device &device, VkDeviceSize bufferSize, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	

	
	void DeinitUniformBuffer(const VKStr::Device &device, VKStr::UniformBuffer * uniformBuffer);

	//descriptor and sempler //настройка ходов для передачи данных в шейдер

	
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


	//Pipelines (конвееры)  //тут подгружаются шейдеры, так же настройка графических элементов

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

	
	//image работа с текстурками 
	void CmdImageLayoutTransition(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);

	void CmdDepthLayoutTransition(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);

	void CmdImageCopy(VkCommandBuffer cmdBuffer, VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);
	

public:

	/**
	* Приостановка рендеринга с ожиданием завершения выполнения всех команд
	*/
	void Pause();

	/**
	* Возврат к состоянию "рендерится"
	*/
	void Continue();

	/**
	* Данный метод вызывается при смене разрешения поверхности отображения
	* либо (в дальнейшем) при смене каких-либо иных настроек графики. В нем происходит
	* пересоздание swap-chain'а и всего того, что зависит от измененных настроек
	*/
	void VideoSettingsChanged();

	/**
	* В методе отрисовки происходит отправка подготовленных команд а так-же показ
	* готовых изображение на поверхности показа
	*/
	void Draw();

	/**
	* В методе обновления происходит отправка новых данных в UBO буферы, то есть
	* учитываются положения камеры, отдельных примитивов и сцены в целом
	*/
	void Update();

	// Настройка параметров перспективы камеры (угол обзора, грани отсечения)
	
	void SetCameraPerspectiveSettings(float fFOV, float fNear, float fFar);

	// Настройка положения камеры
	
	void SetCameraPosition(float x, float y, float z);

	// Настройка поворота камеры
	
	void SetCameraRotation(float x, float y, float z);
	glm::vec3 CameraPos();

	// Добавление нового примитива
	
	unsigned int AddPrimitive(const std::vector<VKStr::Vertex> &vertices,const std::vector<unsigned int> &indices, 
		const VKStr::Texture *texture, int pipelineIndex, glm::vec3 position, glm::vec3 rotaton, glm::vec3 scale = { 1.0f,1.0f,1.0f });
	void UpdatePrimitive(unsigned int primitieInc, glm::vec3 position, glm::vec3 rotaton, glm::vec3 scale = { 1.0f,1.0f,1.0f });
	//свет
	unsigned int initLight(glm::vec3 position, glm::vec3 LightColor, float ambientRate, float diffuseRate, float specularRate, float constant = 1.0f, float linear = 0.007f, float quadratic = 0.0002f);
	void reInitLight(glm::vec3 position, int IDLight);
	void DeleteLight(int IDLight);
	// Создание текстуры 
	VKStr::Texture CreateTexture(const unsigned char* pixels, uint32_t width, uint32_t height, uint32_t channels, uint32_t bpp = 4);
	

	// Конструктор рендерера
	MainRenderer(HINSTANCE hInstance, HWND hWnd, unsigned int primitivesMaxCount = 100);

	
	~MainRenderer();
};



#endif // !1