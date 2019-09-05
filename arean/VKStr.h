#pragma once
#include <string>

//��� �������� ������� ��������� � ������� �������� ���������, ��� ��� � �������� �� �� ���������

namespace VKStr {
	//callback for Validation Layers
	VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallBack(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData);

	
	struct QueueFamilyInfo {

		// �������� �������� ��������
		int graphics = -1;
		int present = -1;
		int compute = -1;
		int transfer = -1;

		// ��������� �� ����� ��������� �������� � �����������
		bool IsRenderingCompatible() const {
			return graphics >= 0 && present >= 0;
		}
	};
	
	//device
	struct Device
	{
		// ���������� � ���������� ����������
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice logicalDevice = VK_NULL_HANDLE;

		// ���������� � ��������� ���������� ��������
		
		QueueFamilyInfo queueFamilies = {};

		// ������ ������������ ��������
		// ����������� �� ����� ������������� ����������
		struct {
			VkQueue graphics = VK_NULL_HANDLE;
			VkQueue present = VK_NULL_HANDLE;
		} queues;

		// ��������� ���������� � ���������� ����������
	
		VkPhysicalDeviceProperties GetProperties() const {
			VkPhysicalDeviceProperties properties = {};
			if (this->physicalDevice != VK_NULL_HANDLE) {
				vkGetPhysicalDeviceProperties(this->physicalDevice, &properties);
			}
			return properties;
		}

		// ������������������� �� ����������?
		bool IsReady() const {
			return this->physicalDevice != VK_NULL_HANDLE &&
				this->logicalDevice != VK_NULL_HANDLE &&
				this->queues.graphics != VK_NULL_HANDLE &&
				this->queues.present != VK_NULL_HANDLE &&
				this->queueFamilies.IsRenderingCompatible();
		}

		// ��������������� ����������
		void Deinit() {
			if (this->logicalDevice != VK_NULL_HANDLE) {
				vkDestroyDevice(this->logicalDevice, nullptr);
				this->logicalDevice = VK_NULL_HANDLE;
			}
			this->physicalDevice = VK_NULL_HANDLE;
			this->queues.graphics = VK_NULL_HANDLE;
			this->queues.present = VK_NULL_HANDLE;
			this->queueFamilies = {};
		}

		// �������� ������������ ������ ��� ����������� ���� �����, �������� ���������� ������ ����������� ����������
		template <typename T>
		VkDeviceSize GetDynamicAlignment() const {
			VkDeviceSize minUboAlignment = this->GetProperties().limits.minUniformBufferOffsetAlignment;
			VkDeviceSize dynamicAlignment = (VkDeviceSize)sizeof(T);

			if (minUboAlignment > 0) {
				dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
			}

			return dynamicAlignment;
		}

		// ��������� ������������ �� ���������� �������� �������-��������� ��� ����������� �������
		bool IsDepthFormatSupported(VkFormat format) const {

			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);

			// ������ ������ ������������ �������� �������-���������
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
				return true;
			}

			return false;
		}

	};

	//surface
	struct SurfaceInfo {

		// ����������� �����������, �������, ������ �������������
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		// �������������� �� ���������� ������ ����������� 
		bool IsSurfaceFormatSupported(VkSurfaceFormatKHR surfaceFormat) const {
			return this->IsFormatSupported(surfaceFormat.format) && this->IsColorSpaceSupported(surfaceFormat.colorSpace);
		}

		// ��������������� �� ���������� �������� ������������
		bool IsColorSpaceSupported(VkColorSpaceKHR colorSpace) const {
			if (this->formats.size() == 1 && this->formats[0].format == VK_FORMAT_UNDEFINED) {
				return true;
			}
			else if (this->formats.size() > 1) {
				for (const VkSurfaceFormatKHR& formatEntry : this->formats) {
					if (formatEntry.colorSpace == colorSpace) {
						return true;
					}
				}
			}
			return false;
		}

		// �������������� �� ���������� ������
		bool IsFormatSupported(VkFormat format) const {
			if (this->formats.size() == 1 && this->formats[0].format == VK_FORMAT_UNDEFINED) {
				return true;
			}
			else if (this->formats.size() > 1) {
				for (const VkSurfaceFormatKHR& formatEntry : this->formats) {
					if (formatEntry.format == format) {
						return true;
					}
				}
			}
			return false;
		}
	};

	//image (need to swapchain)
	struct Image
	{
		VkImage vkImage = VK_NULL_HANDLE;
		VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;
		VkImageView vkImageView = VK_NULL_HANDLE;
		VkFormat format = {};
		VkExtent3D extent = {};

		// ��������������� (������� ������)
		void Deinit(VkDevice logicalDevice) {

			this->format = {};
			this->extent = {};

			if (this->vkImageView != VK_NULL_HANDLE) {
				vkDestroyImageView(logicalDevice, this->vkImageView, nullptr);
				this->vkImageView = VK_NULL_HANDLE;
			}

			if (this->vkImage != VK_NULL_HANDLE) {
				vkDestroyImage(logicalDevice, this->vkImage, nullptr);
				this->vkImage = VK_NULL_HANDLE;
			}

			if (this->vkDeviceMemory != VK_NULL_HANDLE) {
				vkFreeMemory(logicalDevice, this->vkDeviceMemory, nullptr);
				this->vkDeviceMemory = VK_NULL_HANDLE;
			}
		}
	};


	//swapchain
	struct Swapchain {
		// ����� swap-chain
		VkSwapchainKHR vkSwapchain = VK_NULL_HANDLE;

		// ������ ����������� � ����� ��� �������� �������� �����-�������
		// � ��� �� ������ � ����������
		std::vector<VkImage> images;
		std::vector<VkImageView> imageViews;
		VkFormat imageFormat = {};
		VkExtent2D imageExtent = {};

		// ����������� ������ �������-��������� (Z-�����)
		Image depthStencil = {};

		// ������ ������������
		std::vector<VkFramebuffer> framebuffers;
	};


	//buffers 
	struct Buffer {
		VkBuffer vkBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vkDeviceMemory = VK_NULL_HANDLE;
		VkDeviceSize size = 0;
	};

	
	//����� ������ - (���������� �������� ���-�� ������)
	
	struct VertexBuffer : Buffer
	{
		uint32_t count = 0;
	};

	
	// ����� �������� (���������� �������� ���-�� ��������)
	
	struct IndexBuffer : Buffer
	{
		uint32_t count = 0;
	};

	//uniform buffer
	struct UniformBuffer : Buffer
	{
		// ���������� ��� �������������� ������ (������������ ��� ������������� ������)
		VkDescriptorBufferInfo descriptorBufferInfo = {};

		// ��������� �� ����������� ������ ������
		void * pMapped = nullptr;

		// ��������� ������ (����� ����� ��������� pMapped ����� ��������� �� ���)
		VkResult map(VkDevice device, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) {
			return vkMapMemory(device, this->vkDeviceMemory, offset, size, 0, &(this->pMapped));
		}

		// �������� �������� (�������� ��������� �� ������)
		void unmap(VkDevice device) {
			if (this->pMapped) {
				vkUnmapMemory(device, this->vkDeviceMemory);
			}
		}

		// ������������ �����������
		// ��������� ����� ������ ������ ������ ����� �������� ����������� (�������� ��� �������)
		void configDescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0)
		{
			this->descriptorBufferInfo.offset = offset;
			this->descriptorBufferInfo.buffer = this->vkBuffer;
			this->descriptorBufferInfo.range = size;
		}
	};

	//������� �������� ������������ (��� �������������� � �������)
	struct UboWorld
	{
		glm::mat4 worldMatrix = {};
		glm::mat4 viewMatrix = {};
		glm::mat4 projectionMatrix = {};
	};

	typedef glm::mat4 * UboModelArray;

	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory memory;
		VkImageView view;
	};

	struct ShadowPass
	{
		int32_t width, height;
		VkFramebuffer frameBuffer;
		FrameBufferAttachment depth;
		VkRenderPass renderPass;
		VkSampler depthSampler;
		VkDescriptorImageInfo descriptor;
		VkFormat depthFormat;
	};

	struct UBOVectorsLight {
		glm::vec4 outVec1;
		glm::vec4 outVec2;
		glm::vec4 outvec3;
		glm::vec4 outvec4;
		glm::mat4 LightProjection;
		glm::mat4 LightView;
		glm::mat4 LightWorld;
	};



	struct FSInfo { //�� ������ ��� �������� ��������� � 16 ���, ����� 4 �������� �� 4 ����, ��� ������4 �� 16
		glm::vec4 CamPos;
		uint32_t LCount = 0;
	};

	

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 color;
		glm::vec2 texCoord;
		glm::uint32 textureUsed;
		glm::vec3 normals = {};
	};

	struct Synchronization
	{
		VkSemaphore readyToRender = VK_NULL_HANDLE;
		VkSemaphore readyToPresent = VK_NULL_HANDLE;
	};

	struct Texture
	{
		VKStr::Image image = {};
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		void Deinit(VkDevice logicalDevice, VkDescriptorPool descriptorPool) {

			if (descriptorSet != VK_NULL_HANDLE) {
				if (vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &(this->descriptorSet)) != VK_SUCCESS) {
					throw std::runtime_error("Vulkan: Error while destroying descriptor set (texture)");
					this->descriptorSet = VK_NULL_HANDLE;
				}
			}

			image.Deinit(logicalDevice);
		}
	};

	struct Primitive
	{
		bool drawIndexed = true;
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		const Texture * texture;
		glm::vec3 position = {};
		glm::vec3 rotation = {};
		glm::vec3 scale = {};
		int pipelineIndex = 0;
	};

	struct CameraSettings
	{
		// ��������� � ������� ������
		glm::vec3 position = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 roation = glm::vec3(0.0f, 0.0f, 0.0f);

		// ���� ������, ���������, ������� � ������� ����� ���������
		float fFOV = 60.0f;
		float aspectRatio = 1.0f;
		float fNear = 0.1f;
		float fFar = 256.0f;

		// ����������� ������� ��������
		glm::mat4 MakeProjectionMatrix() const {
			glm::mat4 result = glm::perspective(glm::radians(this->fFOV), aspectRatio, fNear, fFar);
			result[1][1] *= -1; //��� y �������������
			return result;
		};

		// ����������� ������� ����
		glm::mat4 MakeViewMatrix() const {
			glm::mat4 rotationMatrix = glm::rotate(glm::mat4(), glm::radians(this->roation.x), glm::vec3(1.0f, 0.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, glm::radians(this->roation.y), glm::vec3(0.0f, 1.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, glm::radians(this->roation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			glm::mat4 translationMatrix = glm::translate(glm::mat4(), this->position);

			return rotationMatrix * translationMatrix;
		};
	};


}