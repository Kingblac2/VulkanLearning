#pragma once
#include "config.h"

namespace vkMesh {

	vk::VertexInputBindingDescription getPosColorBindingDescription() {

		vk::VertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		// x y r g b u v
		bindingDescription.stride = 11 * sizeof(float);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;

		return bindingDescription;
	}

	std::array<vk::VertexInputAttributeDescription, 4> getPosColorAttributeDescriptions() {

		std::array<vk::VertexInputAttributeDescription, 4> attributes;
		
	

		//Pos
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = vk::Format::eR32G32B32Sfloat;
		attributes[0].offset = 0;
		
		//Color
		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = vk::Format::eR32G32B32Sfloat;
		attributes[1].offset = 3 * sizeof(float);

		//TexCoord
		attributes[2].binding = 0;
		attributes[2].location = 2;
		attributes[2].format = vk::Format::eR32G32Sfloat;
		attributes[2].offset = 6 * sizeof(float);

		/*
		* pos          color data
		*  |              |
		* 
		|{0,1,2}|{3,2,1}|{0.2}| | | | | | | | | | | |  |
index:-    1       2       3  4 5 6 7 8 9 .........15 
		*/
		
		// Normals
		attributes[3].binding = 0;
		attributes[3].location = 3;
		attributes[3].format = vk::Format::eR32G32Sfloat;
		attributes[3].offset = 8 * sizeof(float);

		return attributes;

	}
}