#include "frame.h"


#pragma once
#include "config.h"
#include "memory.h"
#include "image.h"

namespace vkUtil {

		void SwapChainFrame::make_descriptor_resources() {

			BufferInput input;
			input.logicalDevice = logicalDevice;
			input.physicalDevice = physicalDevice;
			input.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
			input.size = sizeof(CameraVectors);
			input.usage = vk::BufferUsageFlagBits::eUniformBuffer;
			cameraVectorBuffer = createBuffer(input);

			camerVectorWriteLocation = logicalDevice.mapMemory(cameraVectorBuffer.bufferMemory, 0, sizeof(CameraVectors));

			input.size = sizeof(CameraMatrices);
			cameraMatrixBuffer = createBuffer(input);

			camerMatrixWriteLocation = logicalDevice.mapMemory(cameraMatrixBuffer.bufferMemory, 0, sizeof(CameraMatrices));

			input.size = 1024 * sizeof(glm::mat4);
			input.usage = vk::BufferUsageFlagBits::eStorageBuffer;
			modelBuffer = createBuffer(input);

			modelBufferWriteLocation = logicalDevice.mapMemory(modelBuffer.bufferMemory, 0, 1024 * sizeof(glm::mat4));

			modelTransforms.reserve(1024);
			for (int i = 0; i < 1024; ++i)
			{
				modelTransforms.push_back(glm::mat4(1.0f));
			}


			cameraVectorDescriptor.buffer = cameraVectorBuffer.buffer;
			cameraVectorDescriptor.offset = 0;
			cameraVectorDescriptor.range = sizeof(CameraVectors); //!!!!!
			
			cameraMatrixDescriptor.buffer = cameraMatrixBuffer.buffer;
			cameraMatrixDescriptor.offset = 0;
			cameraMatrixDescriptor.range = sizeof(CameraMatrices);

			modelBufferDescriptor.buffer = modelBuffer.buffer;
			modelBufferDescriptor.offset = 0;
			modelBufferDescriptor.range = 1024 * sizeof(glm::mat4);
		}

		void SwapChainFrame::make_depth_resources()
		{
			depthFormat = vkImage::find_supported_format(
				physicalDevice,
				{ vk::Format::eD32Sfloat ,vk::Format::eD24UnormS8Uint },
				vk::ImageTiling::eOptimal,
				vk::FormatFeatureFlagBits::eDepthStencilAttachment
			);

			vkImage::ImageInputChunk imageInfo;
			imageInfo.logicalDevice = logicalDevice;
			imageInfo.physicalDevice = physicalDevice;
			imageInfo.tiling = vk::ImageTiling::eOptimal;
			imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
			imageInfo.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
			imageInfo.width = width;
			imageInfo.height = height;
			imageInfo.format = depthFormat;
			imageInfo.arrayCount = 1;
			depthBuffer = vkImage::make_image(imageInfo);
			depthBufferMemory = vkImage::make_image_memory(imageInfo, depthBuffer);
			depthBufferView = vkImage::make_image_view(
				logicalDevice, depthBuffer, depthFormat, vk::ImageAspectFlagBits::eDepth,
				vk::ImageViewType::e2D, 1
			);
		}

		void SwapChainFrame::write_descriptor_set()
		{
			

			logicalDevice.updateDescriptorSets(writeOps, nullptr);

			
		}

		void SwapChainFrame::record_write_operations()
		{
			vk::WriteDescriptorSet cameraVectorWrite;
			cameraVectorWrite.dstSet = descriptorSet[pipelineTypes::SKY];
			cameraVectorWrite.dstBinding = 0;
			cameraVectorWrite.dstArrayElement = 0;
			cameraVectorWrite.descriptorCount = 1;
			cameraVectorWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
			cameraVectorWrite.pBufferInfo = &cameraVectorDescriptor;
			
			vk::WriteDescriptorSet cameraMatrixWrite;
			cameraMatrixWrite.dstSet = descriptorSet[pipelineTypes::STANDARD];
			cameraMatrixWrite.dstBinding = 0;
			cameraMatrixWrite.dstArrayElement = 0;
			cameraMatrixWrite.descriptorCount = 1;
			cameraMatrixWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
			cameraMatrixWrite.pBufferInfo = &cameraMatrixDescriptor;


			vk::WriteDescriptorSet ssboWrite;
			ssboWrite.dstSet = descriptorSet[pipelineTypes::STANDARD];
			ssboWrite.dstBinding = 1;
			ssboWrite.dstArrayElement = 0;
			ssboWrite.descriptorCount = 1;
			ssboWrite.descriptorType = vk::DescriptorType::eStorageBuffer;
			ssboWrite.pBufferInfo = &modelBufferDescriptor;

			writeOps = { {cameraVectorWrite, cameraMatrixWrite,  ssboWrite} };
		}

		void SwapChainFrame::destroy() {

			logicalDevice.destroyImage(depthBuffer);
			logicalDevice.freeMemory(depthBufferMemory);
			logicalDevice.destroyImageView(depthBufferView);

			logicalDevice.destroyImageView(imageView);
			logicalDevice.destroyFramebuffer(framebuffer[pipelineTypes::SKY]);
			logicalDevice.destroyFramebuffer(framebuffer[pipelineTypes::STANDARD]);
			logicalDevice.destroyFence(inFlight);
			logicalDevice.destroySemaphore(imageAvailable);
			logicalDevice.destroySemaphore(renderFinished);

			logicalDevice.unmapMemory(cameraVectorBuffer.bufferMemory);
			logicalDevice.freeMemory(cameraVectorBuffer.bufferMemory);
			logicalDevice.destroyBuffer(cameraVectorBuffer.buffer);
		
			logicalDevice.unmapMemory(cameraMatrixBuffer.bufferMemory);
			logicalDevice.freeMemory(cameraMatrixBuffer.bufferMemory);
			logicalDevice.destroyBuffer(cameraMatrixBuffer.buffer);
			
			logicalDevice.unmapMemory(modelBuffer.bufferMemory);
			logicalDevice.freeMemory(modelBuffer.bufferMemory);
			logicalDevice.destroyBuffer(modelBuffer.buffer);
		}
	
}