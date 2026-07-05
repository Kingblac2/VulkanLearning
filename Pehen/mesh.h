#pragma once
#include "config.h"

namespace vkMesh {

	vk::VertexInputBindingDescription getPosColorBindingDescription();

	std::vector<vk::VertexInputAttributeDescription> getPosColorAttributeDescriptions();
}