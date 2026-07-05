#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "image.h"
#include "memory.h"
#include "logging.h"
#include "descriptor.h"
#include "single_time_commands.h"


vk::Image vkImage::make_image(ImageInputChunk input) {

	vk::ImageCreateInfo imageInfo;
	imageInfo.flags = vk::ImageCreateFlagBits() | input.flags;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.extent = vk::Extent3D(input.width, input.height, 1);
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = input.arrayCount;
	imageInfo.format = input.format;
	imageInfo.tiling = input.tiling;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.usage = input.usage;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.samples = vk::SampleCountFlagBits::e1;

	try {

	return input.logicalDevice.createImage(imageInfo);
	
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::get_logger()->print("Unable to make image");
	}

}

vk::DeviceMemory vkImage::make_image_memory(ImageInputChunk input, vk::Image image) {

	vk::MemoryRequirements requirments = input.logicalDevice.getImageMemoryRequirements(image);

	vk::MemoryAllocateInfo allocation;
	allocation.allocationSize = requirments.size;
	allocation.memoryTypeIndex = vkUtil::findMemoryTypeIndex(
		input.physicalDevice, requirments.memoryTypeBits, input.memoryProperties
	);

	try {
		vk::DeviceMemory imageMemory = input.logicalDevice.allocateMemory(allocation);
		input.logicalDevice.bindImageMemory(image, imageMemory, 0);
		return imageMemory;
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::get_logger()->print("Unable to allocate memory for image");
	}
}

void vkImage::transition_image_layout(ImageLayoutTranstionJob job) {

	vkUtil::start_job(job.commandBuffer);

	vk::ImageSubresourceRange access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.baseMipLevel = 0;
	access.levelCount = 1;
	access.baseArrayLayer = 0;
	access.layerCount = job.arrayCount;

	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = job.oldLayout;
	barrier.newLayout = job.newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = job.image;
	barrier.subresourceRange = access;

	vk::PipelineStageFlags sourceStage, dstStage;
	if (job.oldLayout == vk::ImageLayout::eUndefined) {
		barrier.srcAccessMask = vk::AccessFlagBits::eNoneKHR;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else {
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}

	job.commandBuffer.pipelineBarrier(sourceStage, dstStage, vk::DependencyFlags(), nullptr, nullptr, barrier);

	vkUtil::end_job(job.commandBuffer, job.queue);
}

void vkImage::copy_buffer_to_image(BufferImageCopyJob job) {

	vkUtil::start_job(job.commandBuffer);

	vk::BufferImageCopy copy;
	copy.bufferOffset = 0;
	copy.bufferRowLength = 0;
	copy.bufferImageHeight = 0;

	vk::ImageSubresourceLayers access;
	access.aspectMask = vk::ImageAspectFlagBits::eColor;
	access.mipLevel = 0;
	access.baseArrayLayer = 0;
	access.layerCount = job.arrayCount;
	copy.imageSubresource = access;

	copy.imageOffset = vk::Offset3D(0,0,0);
	copy.imageExtent = vk::Extent3D(
		job.width,
		job.height,
		1
	);

	job.commandBuffer.copyBufferToImage(job.srcBuffer, job.dstImage, vk::ImageLayout::eTransferDstOptimal, copy);
	


	vkUtil::end_job(job.commandBuffer, job.queue);
}

vk::ImageView vkImage::make_image_view(vk::Device logicalDevice, vk::Image image, vk::Format format, vk::ImageAspectFlags aspect, vk::ImageViewType type, uint32_t arrayCount)
{

	vk::ImageViewCreateInfo createInfo = {};
	createInfo.image = image;
	createInfo.viewType = type;
	createInfo.components.r = vk::ComponentSwizzle::eIdentity;
	createInfo.components.g = vk::ComponentSwizzle::eIdentity;
	createInfo.components.b = vk::ComponentSwizzle::eIdentity;
	createInfo.components.a = vk::ComponentSwizzle::eIdentity;
	createInfo.subresourceRange.aspectMask = aspect;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = arrayCount;
	createInfo.format = format;

	return  logicalDevice.createImageView(createInfo);
}

vk::Format vkImage::find_supported_format(vk::PhysicalDevice physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for (vk::Format format : candidates) {

		vk::FormatProperties properties = physicalDevice.getFormatProperties(format);


		if (tiling == vk::ImageTiling::eLinear
			&& (properties.linearTilingFeatures & features) == features) {
			return format;
		}
		
		if (tiling == vk::ImageTiling::eOptimal
			&& (properties.optimalTilingFeatures & features) == features) {
			return format;
		}
		
		std::runtime_error("Unable to find suitable format");
	}

}