
#include "mainRenderer.h"

void MainRenderer::CmdImageLayoutTransition(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, 
	VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange)
{
	// Создать барьер памяти изображения
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// В зависимоти от старого (исходного) размещения меняется исходная маска доступа
	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imageMemoryBarrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	}

	// В зависимости от нового (целевого) размещения меняется целевая маска доступа
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}

	// Разметить барьер на верише конвейера (в самом начале)
	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	// Отправить команду барьера в командный буфер
	vkCmdPipelineBarrier(
		cmdBuffer,
		srcStageFlags,
		destStageFlags,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);
}

void MainRenderer::CmdDepthLayoutTransition(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange)
{
	// Создать барьер памяти изображения
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// В зависимоти от старого (исходного) размещения меняется исходная маска доступа
	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imageMemoryBarrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	}

	// В зависимости от нового (целевого) размещения меняется целевая маска доступа
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	}

	// Разметить барьер на верише конвейера (в самом начале)
	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
		VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

	// Отправить команду барьера в командный буфер
	vkCmdPipelineBarrier(
		cmdBuffer,
		srcStageFlags,
		destStageFlags,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier);
}

void MainRenderer::CmdImageCopy(VkCommandBuffer cmdBuffer, VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height)
{
	// Описание слоев подресурса (мип-уровни не используются)
	VkImageSubresourceLayers subresourceLayers = {};
	subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceLayers.baseArrayLayer = 0;
	subresourceLayers.mipLevel = 0;
	subresourceLayers.layerCount = 1;

	// Конфигурация копирования
	VkImageCopy region = {};
	region.srcSubresource = subresourceLayers;
	region.dstSubresource = subresourceLayers;
	region.srcOffset = { 0, 0, 0 };
	region.dstOffset = { 0, 0, 0 };
	region.extent.width = width;
	region.extent.height = height;
	region.extent.depth = 1;

	// Записать команду копирования в буфер
	vkCmdCopyImage(
		cmdBuffer,
		srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &region
	);
}