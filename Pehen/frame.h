#pragma once
#include "config.h"
#include "memory.h"
#include <unordered_map>

namespace vkUtil {

	struct CameraMatrices {
		glm::mat4	view;
		glm::mat4	projection;
		glm::mat4	viewProjection;
	};
	
	struct CameraVectors {
		glm::vec4	forwards;
		glm::vec4	right;
		glm::vec4	up;
	};

	class SwapChainFrame {
	public:
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;

		//swapchain
		vk::Image image;
		vk::ImageView imageView;
		std::unordered_map<pipelineTypes,vk::Framebuffer> framebuffer;

		vk::Image depthBuffer;
		vk::DeviceMemory depthBufferMemory;
		vk::ImageView depthBufferView;
		vk::Format depthFormat;
		int width, height;
		
		vk::CommandBuffer commandBuffer;
		
		// synchronization
		vk::Semaphore imageAvailable, renderFinished;
		vk::Fence inFlight;
		
		//resources
		CameraMatrices cameraMatrixData;
		Buffer cameraMatrixBuffer;
		void* camerMatrixWriteLocation;
		
		CameraVectors cameraVectorData;
		Buffer cameraVectorBuffer;
		void* camerVectorWriteLocation;

		std::vector<glm::mat4> modelTransforms;
		Buffer modelBuffer;
		void* modelBufferWriteLocation;

		//resources descriptors
		vk::DescriptorBufferInfo cameraVectorDescriptor, cameraMatrixDescriptor, modelBufferDescriptor;
		std::unordered_map<pipelineTypes,vk::DescriptorSet> descriptorSet;

		//Write Ops
		std::vector<vk::WriteDescriptorSet> writeOps;
		
		void make_descriptor_resources();

		void make_depth_resources();

		void write_descriptor_set();

		void record_write_operations();

		void destroy();
		
	}; 
}