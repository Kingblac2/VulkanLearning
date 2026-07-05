#pragma once
#include "config.h"
#include "frame.h"
#include<sstream>
namespace vkInit {

	struct framebufferInput {
		vk::Device device;
		std::unordered_map<pipelineTypes,vk::RenderPass> renderpass;
		vk::Extent2D swapchainExtent;
	};

	void make_framebuffers(framebufferInput inputChunk, std::vector<vkUtil::SwapChainFrame>& frames, bool debug) {


		std::stringstream message;

		//Sky Framebuffer
		for (int i = 0; i < frames.size(); ++i)
		{
			std::vector<vk::ImageView> attachments = {
				frames[i].imageView
			};
			vk::FramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.flags = vk::FramebufferCreateFlags();
			framebufferInfo.renderPass = inputChunk.renderpass[pipelineTypes::SKY];
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = inputChunk.swapchainExtent.width;
			framebufferInfo.height = inputChunk.swapchainExtent.height;
			framebufferInfo.layers = 1;

			try {
				frames[i].framebuffer[pipelineTypes::SKY] = inputChunk.device.createFramebuffer(framebufferInfo);

				if (debug) {
					std::cout << "Created sky framebuffer for frame " << i << std::endl;

				}
						message << "Created sky framebuffer for frame " << i;
						vkLogging::Logger::get_logger()->print(message.str());
						message.str("");
			}
			catch (vk::SystemError err) {
				if (debug)
				{
					std::cout << "Failed to create sky framebuffer for frame " << i << std::endl;
				}
				message << "Failed to create sky framebuffer for frame " << i;
				vkLogging::Logger::get_logger()->print(message.str());
				message.str("");

			}

			//Default-Standrad Frame Buffer

			attachments.push_back(frames[i].depthBufferView);

			framebufferInfo.renderPass = inputChunk.renderpass[pipelineTypes::STANDARD];
			framebufferInfo.attachmentCount = attachments.size();
			framebufferInfo.pAttachments = attachments.data();


			try {
				frames[i].framebuffer[pipelineTypes::STANDARD] = inputChunk.device.createFramebuffer(framebufferInfo);

				if (debug) {
					std::cout << "Created standard framebuffer for frame " << i << std::endl;
				}

				message << "Created standard framebuffer for frame " << i;
				vkLogging::Logger::get_logger()->print(message.str());
				message.str("");
			}
			catch (vk::SystemError err) {
				if (debug)
				{
					std::cout << "Failed to create standard framebuffer for frame " << i << std::endl;
				}
				message << "Failed to create standard framebuffer for frame " << i;
				vkLogging::Logger::get_logger()->print(message.str());
				message.str("");
			}
		}
	}
}