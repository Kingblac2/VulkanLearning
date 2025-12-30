#pragma once
#include"config.h"
#include "logging.h"
#include"queue_families.h"
#include "frame.h"
#include "image.h"
namespace vkInit {


	struct SwapChainSupportDetails {
		vk::SurfaceCapabilitiesKHR capablities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};


	struct SwapChainBundle {
		vk::SwapchainKHR swapchain;
		std::vector<vkUtil::SwapChainFrame> frames;
		vk::Format format;
		vk::Extent2D extent;
	};

	SwapChainSupportDetails query_swapchain_support(vk::PhysicalDevice device, vk::SurfaceKHR surface, bool debug)
	{
		SwapChainSupportDetails support;

		support.capablities = device.getSurfaceCapabilitiesKHR(surface);

		// Check the capabilities in VKSurfaceCappablitesKhR Struct

		if (debug) {
			std::cout << "Swapchain  can support the following surface capablities:\n";

			std::cout << "\tminimum image count: " << support.capablities.minImageCount << std::endl;
			std::cout << "\tmacimum image count: " << support.capablities.maxImageCount << std::endl;

			std::cout << "\tcurrent extent: " << std::endl;

			std::cout << "\t\twidth:  " << support.capablities.currentExtent.width << std::endl;
			std::cout << "\t\theight:  " << support.capablities.currentExtent.height << std::endl;

			std::cout << "\tminimum supported extent: " << std::endl;
			std::cout << "\t\twidth:  " << support.capablities.minImageExtent.width << std::endl;
			std::cout << "\t\theight:  " << support.capablities.minImageExtent.height << std::endl;

			std::cout << "\tmaximum supported extent: " << std::endl;
			std::cout << "\t\twidth:  " << support.capablities.maxImageExtent.width << std::endl;
			std::cout << "\t\theight:  " << support.capablities.maxImageExtent.height << std::endl;

			std::cout << "\t\tmaximum image array layers:  " << support.capablities.maxImageArrayLayers << std::endl;

			std::cout << "\tsupported transform:\n";
			std::vector<std::string> stringList = log_transform_bits(support.capablities.supportedTransforms);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << '\n';
			}


			std::cout << "\tcurrent transform:\n";
			stringList = log_transform_bits(support.capablities.currentTransform);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << '\n';
			}

			std::cout << "\tsupported alpha operation:\n";
			stringList = log_alpha_composite_bits(support.capablities.supportedCompositeAlpha);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << '\n';
			}

			std::cout << "\tsupported image usage:\n";
			stringList = log_image_usage_bits(support.capablities.supportedUsageFlags);
			for (std::string line : stringList)
			{
				std::cout << "\t\t" << line << '\n';
			}

			support.formats = device.getSurfaceFormatsKHR(surface);

			if (debug)
			{
				for (vk::SurfaceFormatKHR supportedFormat : support.formats)
				{
					std::cout << "supported pixel format: " << vk::to_string(supportedFormat.format) << '\n';
					std::cout << "supported color space: " << vk::to_string(supportedFormat.colorSpace) << '\n';
				}
			}

			support.presentModes = device.getSurfacePresentModesKHR(surface);

			for (vk::PresentModeKHR presentMode : support.presentModes) {
				std::cout << '\t' << log_present_mode(presentMode) << '\n';
			}
		}
		return support;
	}

	vk::SurfaceFormatKHR choose_swapchain_surface_format(std::vector<vk::SurfaceFormatKHR> formats)
	{
		for (vk::SurfaceFormatKHR format : formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Unorm
				&& format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				return format;
			}
		}
		return formats[0];
	}

	vk::PresentModeKHR choose_swapchain_present_mode(std::vector<vk::PresentModeKHR> presentModes) {

		for (vk::PresentModeKHR presentMode : presentModes) {
			if (presentMode == vk::PresentModeKHR::eMailbox) {
				return presentMode;
			}
		}
		return vk::PresentModeKHR::eFifo;
	}


	vk::Extent2D choose_swapchain_extent(uint32_t  width, uint32_t height, vk::SurfaceCapabilitiesKHR capablities)
	{
		if (capablities.currentExtent.width != UINT32_MAX) {
			return capablities.currentExtent;
		}
		else {
			vk::Extent2D extent = { width,height };

			extent.width = std::min(
				capablities.maxImageExtent.width,
				std::max(capablities.minImageExtent.width, width)
			);

			extent.height = std::min(
				capablities.maxImageExtent.height,
				std::max(capablities.minImageExtent.height, height)
			);

			return extent;
		}
	}
	SwapChainBundle create_swapchain(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, int width, int height, bool debug)
	{
		SwapChainSupportDetails support = query_swapchain_support(physicalDevice, surface, debug);

		vk::SurfaceFormatKHR format = choose_swapchain_surface_format(support.formats);

		vk::PresentModeKHR presentMode = choose_swapchain_present_mode(support.presentModes);

		vk::Extent2D extent = choose_swapchain_extent(width, height, support.capablities);

		uint32_t imageCount = std::min(
			support.capablities.maxImageCount,
			support.capablities.minImageCount + 1
		);

		vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR(
			vk::SwapchainCreateFlagsKHR(), surface, imageCount, format.format, format.colorSpace,
			extent, 1, vk::ImageUsageFlagBits::eColorAttachment
		);

		vkUtil::QueueFamilyIndices indicies = vkUtil::findQueueFamilies(physicalDevice, surface, debug);
		uint32_t queueFamilyIndices[] = { indicies.graphicsFamily.value(),indicies.presentFamily.value() };
		if (indicies.graphicsFamily.value() != indicies.presentFamily.value()) {
			createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = vk::SharingMode::eExclusive;
		}

		createInfo.preTransform = support.capablities.currentTransform;
		createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = vk::SwapchainKHR(nullptr);

		SwapChainBundle bundle{};
		try
		{
			bundle.swapchain = logicalDevice.createSwapchainKHR(createInfo);
		}
		catch (vk::SystemError err)
		{
			throw std::runtime_error("failed to create swapchin!");
		}

		std::vector<vk::Image> images = logicalDevice.getSwapchainImagesKHR(bundle.swapchain);
		bundle.frames.resize(images.size());

		for (size_t i = 0; i < images.size(); ++i)
		{
			bundle.frames[i].image = images[i];
			bundle.frames[i].imageView = vkImage::make_image_view(logicalDevice, images[i], format.format, vk::ImageAspectFlagBits::eColor);
		}
		bundle.format = format.format;
		bundle.extent = extent;

		return bundle;
	}
}