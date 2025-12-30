#pragma once
#include "config.h"
#include "memory.h"

class TraingleMesh {
public:
	TraingleMesh(vk::Device logicalDevice, vk::PhysicalDevice physicalDevice);
	~TraingleMesh();
	vkUtil::Buffer vertexBuffer;
private:
	vk::Device logicalDevice;
};