#pragma once
#include "config.h"
#include "queue_families.h"

namespace vkInit {

	struct commandBufferInputChunk {
		vk::Device device;
		vk::CommandPool commandPool;
		std::vector<vkUtil::SwapChainFrame>& frames;
	};

	vk::CommandPool make_command_pool(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surfece, bool debug) {

		vkUtil::QueueFamilyIndices queueFamilyIndices = vkUtil::findQueueFamilies(physicalDevice, surfece, debug);

		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.flags = vk::CommandPoolCreateFlags() | vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

		try {
			return device.createCommandPool(poolInfo);
		} 
		catch (vk::SystemError err) {
			if (debug)
			{ 
				std::cout << "Failed to create Command Pool" << std::endl;
			}
			return nullptr;
		}
	}

	vk::CommandBuffer make_command_buffer(commandBufferInputChunk inputChunk, bool debug)
	{

		vk::CommandBufferAllocateInfo allocinfo = {};
		allocinfo.commandPool = inputChunk.commandPool;
		allocinfo.level = vk::CommandBufferLevel::ePrimary;
		allocinfo.commandBufferCount = 1;



		try {
			vk::CommandBuffer  commandBuffer = inputChunk.device.allocateCommandBuffers(allocinfo)[0];

			if (debug) {
				std::cout << "Allocated main command buffer " << std::endl;
			}
			return commandBuffer;
		}
		catch (vk::SystemError err) {
			if (debug)
			{
				std::cout << " Failed to allocate main command buffer" << std::endl;
			}
			return nullptr;
		}
	}

	void make_frame_command_buffers(commandBufferInputChunk inputChunk, bool debug)
	{

		vk::CommandBufferAllocateInfo allocinfo = {};
		allocinfo.commandPool = inputChunk.commandPool;
		allocinfo.level = vk::CommandBufferLevel::ePrimary;
		allocinfo.commandBufferCount = 1;
		  
		for (int i = 0; i < inputChunk.frames.size(); ++i) {
			try {
				inputChunk.frames[i].commandBuffer = inputChunk.device.allocateCommandBuffers(allocinfo)[0];
			
				if (debug) {
					std::cout << "Allocated command buffer for frame " << i << std::endl;
				}
			}
			catch (vk::SystemError err) {
				if (debug)
				{
					std::cout << " Failed to allocate command buffer for frame " << i << std::endl;
				}
			}
		}


	}
}