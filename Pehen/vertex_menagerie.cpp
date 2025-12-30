#include "vertex_menagerie.h"

VertexMenagerie::VertexMenagerie()
{
	indexOffset = 0;
}

VertexMenagerie::~VertexMenagerie()
{
	logicalDevice.destroyBuffer(vertexBuffer.buffer);
	logicalDevice.freeMemory(vertexBuffer.bufferMemory);

	logicalDevice.destroyBuffer(indexBuffer.buffer);
	logicalDevice.freeMemory(indexBuffer.bufferMemory);
}

void VertexMenagerie::consume(meshTypes type, std::vector<float> vertexData, std::vector<uint32_t> indexData)
{
	
	int vertexCount = static_cast<int>(vertexData.size() / 7);
	int indexCount  = static_cast<int>(indexData.size());
	int lastIndex   = static_cast<int>(indexlump.size());

	firstIndices.insert(std::make_pair(type, lastIndex));
	IndexCounts.insert(std::make_pair(type, indexCount));


	for (float attribute : vertexData) {
		vertexlump.push_back(attribute);
	}
	for (uint32_t index : indexData) {
		indexlump.push_back(index + indexOffset);
	}

	indexOffset += vertexCount;
}

void VertexMenagerie::finalize(FinializationChunk chunk)
{
	this->logicalDevice = chunk.logicalDevice;

	// make a stagging buffer for vertices
	vkUtil::BufferInput inputChunk;
	inputChunk.logicalDevice = chunk.logicalDevice;
	inputChunk.physicalDevice = chunk.physicalDevice;
	inputChunk.size = sizeof(float) * vertexlump.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

	vkUtil::Buffer stagingBuffer = vkUtil::createBuffer(inputChunk);

	// fill it with vertex data
	void* memoryLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, vertexlump.data(), inputChunk.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);
	
	// make the vertex buffer
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;

	vertexBuffer = vkUtil::createBuffer(inputChunk);


	//fill it
	vkUtil::copyBuffer(
		stagingBuffer, vertexBuffer, inputChunk.size, 
		chunk.queue, chunk.commandBuffer
	);

	//destroy stagging buffer
	logicalDevice.destroyBuffer(stagingBuffer.buffer);
	logicalDevice.freeMemory(stagingBuffer.bufferMemory);

	// make a stagging buffer for indices
	inputChunk.size = sizeof(uint32_t) * indexlump.size();
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferSrc;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	stagingBuffer = vkUtil::createBuffer(inputChunk);

	// fill it with index data
	memoryLocation = logicalDevice.mapMemory(stagingBuffer.bufferMemory, 0, inputChunk.size);
	memcpy(memoryLocation, indexlump.data(), inputChunk.size);
	logicalDevice.unmapMemory(stagingBuffer.bufferMemory);

	// make the index buffer
	inputChunk.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer;
	inputChunk.memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
	indexBuffer = vkUtil::createBuffer(inputChunk);

	//fill it
	vkUtil::copyBuffer(
		stagingBuffer, indexBuffer, inputChunk.size,
		chunk.queue, chunk.commandBuffer
	);

	//destroy stagging buffer
	logicalDevice.destroyBuffer(stagingBuffer.buffer);
	logicalDevice.freeMemory(stagingBuffer.bufferMemory);

}