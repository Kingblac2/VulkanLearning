
#include "stb_image.h"
#include "texture.h"
#include "memory.h"
#include "logging.h"
#include "descriptor.h"

vkImage::Texture::Texture(TextureInputChunk info)
{
	logicalDevice = info.logicalDevice;
	physicalDevice = info.physicalDevice;
	filename = info.filenames[0];
	commandBuffer = info.commandBuffer;
	queue = info.queue;
	layout = info.layout;
	descriptorPool = info.descriptorPool;

	load();

	ImageInputChunk imageInput;
	imageInput.logicalDevice = logicalDevice;
	imageInput.physicalDevice = physicalDevice;
	imageInput.height = height;
	imageInput.width = width;
	imageInput.arrayCount = 1;
	imageInput.format = vk::Format::eR8G8B8A8Unorm;
	imageInput.tiling = vk::ImageTiling::eOptimal;
	imageInput.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
	imageInput.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	image = make_image(imageInput);
	imageMemory = make_image_memory(imageInput, image);

	populate();

	free(pixels);

	make_view();

	make_sampler();

	make_descriptor_set();
}

vkImage::Texture::~Texture()
{
	logicalDevice.freeMemory(imageMemory);
	logicalDevice.destroyImage(image);
	logicalDevice.destroyImageView(imageView);
	logicalDevice.destroySampler(sampler);
}

void vkImage::Texture::use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout)
{
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 1, descriptorSet, nullptr);

}

void vkImage::Texture::load()
{
	pixels = ::stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) {
		vkLogging::Logger::get_logger()->print_list({ "Failed to load: ", filename,stbi_failure_reason() });
	}
}

void vkImage::Texture::populate() {

	vkUtil::BufferInput input;
	input.logicalDevice = logicalDevice;
	input.physicalDevice = physicalDevice;
	input.memoryProperties = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
	input.usage = vk::BufferUsageFlagBits::eTransferSrc;
	input.size = width * height * 4;
	vkUtil::Buffer stagingBuffer = vkUtil::createBuffer(input);

	void* writeLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, input.size);
	memcpy(writeLocation, pixels, input.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);

	ImageLayoutTranstionJob transtionJob;
	transtionJob.commandBuffer = commandBuffer;
	transtionJob.queue = queue;
	transtionJob.image = image;
	transtionJob.oldLayout = vk::ImageLayout::eUndefined;
	transtionJob.newLayout = vk::ImageLayout::eTransferDstOptimal;
	transtionJob.arrayCount = 1;
	transition_image_layout(transtionJob);

	BufferImageCopyJob copyJob;
	copyJob.commandBuffer = commandBuffer;
	copyJob.queue = queue;
	copyJob.srcBuffer = stagingBuffer.buffer;
	copyJob.dstImage = image;
	copyJob.width = width;
	copyJob.height = height;
	copyJob.arrayCount = 1;
	copy_buffer_to_image(copyJob);

	transtionJob.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	transtionJob.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	transition_image_layout(transtionJob);

	logicalDevice.freeMemory(stagingBuffer.bufferMemory);
	logicalDevice.destroyBuffer(stagingBuffer.buffer);
}

void vkImage::Texture::make_view()
{
	imageView = make_image_view(logicalDevice, image, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor,vk::ImageViewType::e2D, 1);
}

void vkImage::Texture::make_sampler() {
	vk::SamplerCreateInfo samplerInfo;
	samplerInfo.flags = vk::SamplerCreateFlags();
	samplerInfo.minFilter = vk::Filter::eNearest;
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerInfo.anisotropyEnable = false;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareEnable = false;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	try {
		sampler = logicalDevice.createSampler(samplerInfo);
	}
	catch (vk::SystemError err) {
		vkLogging::Logger::get_logger()->print("Failed to make sampler.");
	}
}

void vkImage::Texture::make_descriptor_set() {

	descriptorSet = vkInit::allocate_descriptor_Set(logicalDevice, descriptorPool, layout);

	vk::DescriptorImageInfo imageDescriptor;
	imageDescriptor.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	imageDescriptor.imageView = imageView;
	imageDescriptor.sampler = sampler;

	vk::WriteDescriptorSet descriptorWrite;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageDescriptor;

	logicalDevice.updateDescriptorSets(descriptorWrite, nullptr);
}