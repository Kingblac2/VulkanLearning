#pragma once
#include"config.h"

namespace vkInit{

	bool supported(std::vector<const char*>& extensions, std::vector<const char*>& layers, bool debug)
	{
		std::vector<vk::ExtensionProperties> supportedExtensions = vk::enumerateInstanceExtensionProperties();

		if (debug)
		{
			std::cout << "Device can support the following extensions:\n";
			for (vk::ExtensionProperties supportedExtension : supportedExtensions)
			{
				std::cout << '\t' << supportedExtension.extensionName << std::endl;
			}
		}

		bool found;
		for (const char* extension : extensions)
		{
			found = false;
			for (vk::ExtensionProperties supportedExtension : supportedExtensions)
			{
				if (strcmp(extension, supportedExtension.extensionName) == 0)
				{
					found = { true };
					if (debug)
					{
						std::cout << "Extension \"" << extension << "\"is supported!\n";
					}
				}
			}
			if (!found)
			{
				if (debug)
				{
					std::cout << "Extension \"" << extension << "\" is not supported!\n";
				}
				return false;
			}
		}

		std::vector<vk::LayerProperties> supportedLayers = vk::enumerateInstanceLayerProperties();

		if (debug)
		{
			std::cout << "Device can support the following Layers:\n";
			for (vk::LayerProperties supportedLayer : supportedLayers)
			{
				std::cout << '\t' << supportedLayer.layerName << std::endl;
			}
		}

		for (const char* layer : layers)
		{
			found = false;
			for (vk::LayerProperties supportedLayer : supportedLayers)
			{
				if (strcmp(layer, supportedLayer.layerName) == 0)
				{
					found = true;
					if (debug)
					{
						std::cout << "Layer \"" << layer << "\" is supported!\n";
					}
				}
			}
			if (!found)
			{
				if (!debug)
				{
					
						std::cout << "Layer \"" << layer << "\" is no supported!\n";
				}
				return false;
			}
		}
		return true;
	}
	vk::Instance make_instance(bool debug, const char* applicationName)
	{
		if (debug)
		{
			std::cout << "Making an instance...\n";
		}

		uint32_t version{ 0 };
		vkEnumerateInstanceVersion(&version);

		if (debug)
		{
			std::cout << "Systems can support vulkan Variant: " << VK_API_VERSION_VARIANT(version)
				<< ", Major:" << VK_API_VERSION_MAJOR(version)
				<< ", Minor:" << VK_API_VERSION_MINOR(version)
				<< ", Patch:" << VK_API_VERSION_PATCH(version) << '\n';
		}
		/*
		* We  can then either use this version
		* (We Should just be sure to set the patch to 0 for best compatibility/stability)
		*/
		// Removes Lower 12 bytes, Where Patch Number is Stored
		version &= ~(0xFFU);

		// Requesting The 0.1.0.0 version
		// Change VK_MAKE_API_VERSION to VK_MAKE_API_VERSION(0,1,0,0) if some problem occurs
		version = VK_MAKE_API_VERSION(0, 1, 1, 0);

		vk::ApplicationInfo appinfo = vk::ApplicationInfo(
			applicationName,
			version,
			"Doing it the hard way",
			version,
			version
		);
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (debug)
		{
			extensions.push_back("VK_EXT_debug_utils");
		}
		if (debug)
		{
			std::cout << "extensions to be requested:\n";

			for (const char* extensionName : extensions)
			{
				std::cout<< "\t\""<<extensionName << "\"\n";
			}
		}

		std::vector<const char*> layers;
		if (debug) layers.push_back("VK_LAYER_KHRONOS_validation");
		if (!supported(extensions, layers, debug))
		{
			return nullptr;
		}

		vk::ValidationFeaturesEXT validationFeatures = {};
		{
			validationFeatures.sType = vk::StructureType::eValidationFeaturesEXT;
			validationFeatures.enabledValidationFeatureCount = 1;
			vk::ValidationFeatureEnableEXT enables[] = {
				vk::ValidationFeatureEnableEXT::eGpuAssisted,
				vk::ValidationFeatureEnableEXT::eGpuAssistedReserveBindingSlot
			};
			validationFeatures.pEnabledValidationFeatures = enables;
		}

		vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo(
			vk::InstanceCreateFlags(),
			&appinfo,
			static_cast<uint32_t>(layers.size()), layers.data(),// enabled layers
			static_cast<uint32_t>(extensions.size()), extensions.data() //enable extension
			,&validationFeatures);

		
			

			
	
		try{
			return vk::createInstance(createInfo);
		}
		catch (vk::SystemError err)
		{
			if (debug) {
				std::cout << "Failed to create instance\n";
			}
		}
		return nullptr;
	}
}