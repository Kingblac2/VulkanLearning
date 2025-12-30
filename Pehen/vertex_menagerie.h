#pragma once
#include "config.h"
#include "memory.h"
#include <unordered_map>


struct FinializationChunk {
	vk::Device logicalDevice;
	vk::PhysicalDevice physicalDevice;
	vk::Queue queue;
	vk::CommandBuffer commandBuffer;
};

class VertexMenagerie {
public:
	VertexMenagerie();
	~VertexMenagerie();
	void  consume(meshTypes type, std::vector<float> vertexData,std::vector<uint32_t> indexData);
	void  finalize(FinializationChunk finalizationChunk);
	vkUtil::Buffer vertexBuffer;
	vkUtil::Buffer indexBuffer;
	std::unordered_map<meshTypes, int> firstIndices;
	std::unordered_map<meshTypes, int> IndexCounts;

private:
	int indexOffset;
	vk::Device logicalDevice;
	std::vector<float> vertexlump;
	std::vector<float> indexlump;

};