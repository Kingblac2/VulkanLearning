#pragma once
#include "config.h"
#include"logging.h"
#include"queue_families.h"
namespace vkInit {




	bool checkDeviceExtensionSupport(
		const vk::PhysicalDevice& device,
		const std::vector<const char*>& requestedExtensions,
		const bool& debug)
	{

		std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

		if (debug) {
			std::cout << "Device can support extensions:\n";
		}


		for (vk::ExtensionProperties& extension : device.enumerateDeviceExtensionProperties()) {

			if (debug) {
				std::cout << "\t\"" << extension.extensionName << "\"\n";
			}
			requiredExtensions.erase(extension.extensionName);

		}

		return requiredExtensions.empty();
	}
	bool isSuitable(const vk::PhysicalDevice& device, const bool debug)
	{
		if (debug) {
			std::cout << "Checking if device is suitable\n";
		}

		const std::vector<const char*> requestedExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		if (debug){
			std::cout << "We are requesting device extensions:\n";

			for(const char* extension : requestedExtensions)
			{
				std::cout << "\t\"" << extension << "\"\n";
			}


		}
		if (bool extensionsSupported = checkDeviceExtensionSupport(device,requestedExtensions,debug)){
				if (debug) {
					std::cout << "Device can support the requested extensions!\n";
				}
		}
		else
		{
				if (debug) {
					std::cout << "Device can't support thr requested extension\n";
				}
				return false;
		}
		return true;
	}

	vk::PhysicalDevice choose_physical_device(vk::Instance& instance, bool debug)
	{	
		if (debug)
		{
			std::cout << "Choosing physical device....\n";
		}

		std::vector<vk::PhysicalDevice> availableDevices = instance.enumeratePhysicalDevices();

		if (debug)
		{
			std::cout << "There are " << availableDevices.size() << " physical device avilables  on this system\n";
		}


		for (auto device : availableDevices)
		{
			if (debug){
				log_device_properties(device);
			}
			if (isSuitable(device, debug))
			{
				return device;
			}
		}

		return nullptr;
	}

	
	vk::Device create_logical_device(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool debug) {

		 vkUtil::QueueFamilyIndices indices = vkUtil::findQueueFamilies(physicalDevice, surface, debug);
		std::vector<uint32_t> uniqueIndices;
		uniqueIndices.push_back(indices.graphicsFamily.value());
		if (indices.graphicsFamily.value() != indices.presentFamily.value())
		{
			uniqueIndices.push_back(indices.presentFamily.value());
		}

		float queuePriority = 1.0f;
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfo;
		for (uint32_t queueFamilyIndex : uniqueIndices)
		{
			queueCreateInfo.push_back(vk::DeviceQueueCreateInfo(
				vk::DeviceQueueCreateFlags(), queueFamilyIndex,
				1, &queuePriority
			));
		}

		std::vector<const char*> device_extension = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		vk::PhysicalDeviceFeatures deviceFeatures = vk::PhysicalDeviceFeatures();
		std::vector<const char*> enabledLayers;
		if (debug)
		{
			enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
		}
		vk::DeviceCreateInfo deviceInfo = vk::DeviceCreateInfo(
			vk::DeviceCreateFlags(),
			queueCreateInfo.size(), queueCreateInfo.data(),
			enabledLayers.size(), enabledLayers.data(),
			device_extension.size(), device_extension.data(),
			&deviceFeatures

		);

		try
		{
			vk::Device device = physicalDevice.createDevice(deviceInfo);
			if (debug) {
				std::cout << "GPU has been successfully abstracted!\n";
			}
			return device;
		}
		catch (vk::SystemError err)
		{
			if (debug)
			{
				std::cout << "Device CreationFail!\n" << err.what();
				return nullptr;
			}
		}
		return nullptr;
	}
	std::array<vk::Queue, 2> get_queue(vk::PhysicalDevice physicalDevice, vk::Device device, vk::SurfaceKHR surface, bool debug) {
	vkUtil::QueueFamilyIndices indices = vkUtil::findQueueFamilies(physicalDevice, surface, device);

		return{ {
				device.getQueue(indices.graphicsFamily.value(), 0),
				device.getQueue(indices.presentFamily.value(), 0)
				} };
	}

}