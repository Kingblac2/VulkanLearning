#include"descriptor.h"



vk::DescriptorSetLayout vkInit::make_descriptor_set_layout(vk::Device device, const descriptorSetLayoutData& bindings) {

		std::vector<vk::DescriptorSetLayoutBinding> layoutBindings;
		layoutBindings.reserve(bindings.count);

		for (int i = 0; i < bindings.count; i++)
		{
			vk::DescriptorSetLayoutBinding layoutBinding;
			layoutBinding.binding = bindings.indices[i];
			layoutBinding.descriptorType = bindings.types[i];
			layoutBinding.descriptorCount = bindings.counts[i];
			layoutBinding.stageFlags = bindings.stages[i];
			layoutBindings.push_back(layoutBinding);
		}
		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits();
		layoutInfo.bindingCount = bindings.count;
		layoutInfo.pBindings = layoutBindings.data();

		try {
			return device.createDescriptorSetLayout(layoutInfo);
		}
		catch (vk::SystemError err)
		{
			return nullptr;
		}
	}

	vk::DescriptorPool vkInit::make_descriptor_pool(vk::Device device, uint32_t size, const descriptorSetLayoutData& bindings)
	{

		std::vector<vk::DescriptorPoolSize> poolSizes;

		for (int i = 0; i < bindings.count; i++)
		{
			vk::DescriptorPoolSize poolSize;
			poolSize.type = bindings.types[i];
			poolSize.descriptorCount = 10;
			poolSizes.push_back(poolSize);
		}

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.flags = vk::DescriptorPoolCreateFlags();
		poolInfo.maxSets = size;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		try {
			return device.createDescriptorPool(poolInfo);
		}
		catch (vk::SystemError err) {
			if (_DEBUG)
			{
				std::cout << "Failed to make descriptor pool";
			}
			return nullptr;
		}

	}

	vk::DescriptorSet vkInit::allocate_descriptor_Set(vk::Device device, vk::DescriptorPool descriptorPool, vk::DescriptorSetLayout layout)
	{
		vk::DescriptorSetAllocateInfo allocationInfo;

		allocationInfo.descriptorPool = descriptorPool;
		allocationInfo.descriptorSetCount = 1;
		allocationInfo.pSetLayouts = &layout;

		try {
			return device.allocateDescriptorSets(allocationInfo)[0];
		}
		catch (vk::SystemError err)
		{
			if (_DEBUG)
			{
				std::cout << "Failed to allocate descriptor set from pool";
			}
			return nullptr;
		}

	}
