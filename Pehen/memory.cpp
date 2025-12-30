#include "memory.h"
#include"single_time_commands.h"


namespace vkUtil {



	Buffer createBuffer(BufferInput input) {

		vk::BufferCreateInfo bufferInfo;
		bufferInfo.flags = vk::BufferCreateFlags();
		bufferInfo.size = input.size;
		bufferInfo.usage = input.usage;
		bufferInfo.sharingMode = vk::SharingMode::eExclusive;

		Buffer buffer;
		buffer.buffer = input.logicalDevice.createBuffer(bufferInfo);

		allocateBufferMemory(buffer, input);

		return buffer;

	}

	uint32_t findMemoryTypeIndex(vk::PhysicalDevice physicalDevice, uint32_t supportedMemoryIndices, vk::MemoryPropertyFlags requestedProperties) {

		vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {

			bool supported{ static_cast<bool>(supportedMemoryIndices & (1 << i)) };

			bool sufficient{ (memoryProperties.memoryTypes[i].propertyFlags & requestedProperties) == requestedProperties };

			if (supported && sufficient) {
				return i;
			}
		}


	}

	void allocateBufferMemory(Buffer& buffer, const BufferInput& input) {

		vk::MemoryRequirements memoryRequirments = input.logicalDevice.getBufferMemoryRequirements(buffer.buffer);

		vk::MemoryAllocateInfo allocInfo;
		allocInfo.allocationSize = memoryRequirments.size;
		allocInfo.memoryTypeIndex = findMemoryTypeIndex(
			input.physicalDevice, memoryRequirments.memoryTypeBits,
			input.memoryProperties
		);

		buffer.bufferMemory = input.logicalDevice.allocateMemory(allocInfo);
		input.logicalDevice.bindBufferMemory(buffer.buffer, buffer.bufferMemory, 0);
	}
	void copyBuffer(Buffer& srcBuffer, Buffer& dstBuffer, vk::DeviceSize size, vk::Queue queue, vk::CommandBuffer commandBuffer)
	{
		start_job(commandBuffer);

		vk::BufferCopy copyRegion;
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		commandBuffer.copyBuffer(srcBuffer.buffer, dstBuffer.buffer,1,  &copyRegion);

		end_job(commandBuffer, queue);
	}
}