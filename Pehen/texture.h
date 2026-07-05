#pragma once

#include "stb_image.h"

#include "config.h"

#include "image.h"
namespace vkImage {


	class Texture {

	public:

		Texture(TextureInputChunk info);

		void use(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout);

		~Texture();

	private:

		int width, height, channels;
		vk::Device logicalDevice;
		vk::PhysicalDevice physicalDevice;
		const char* filename;
		stbi_uc* pixels;

		//Resources
		vk::Image image;
		vk::DeviceMemory imageMemory;
		vk::ImageView imageView;
		vk::Sampler sampler;

		//Resource Descriptors (for sampler probably)
		vk::DescriptorSetLayout layout;
		vk::DescriptorSet descriptorSet;
		vk::DescriptorPool descriptorPool;

		vk::CommandBuffer commandBuffer;
		vk::Queue queue;

		void load();

		void populate();

		void make_view();

		void make_sampler();

		void make_descriptor_set();
	};

}
