#pragma once
#include "config.h"


namespace vkUtil {

	struct BufferInput {
		size_t size;
		vk::BufferUsageFlags usage;
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		vk::MemoryPropertyFlags memoryProperties;
	};

	struct Buffer {
		vk::Buffer buffer;
		vk::DeviceMemory bufferMemory;

	};

	Buffer createBuffer(BufferInput input);

	uint32_t findMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties);

	void allocateBufferMemory(Buffer& buffer, const BufferInput& input);

	void copyBuffer(Buffer& buffer, Buffer& dstBuffer, vk::DeviceSize size, vk::Queue queue, vk::CommandBuffer commandBuffer);
}