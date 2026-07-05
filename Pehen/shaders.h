#pragma once
#include"config.h"
 
namespace vkUtil {

	std::vector<char> readFile(std::string filename, bool debug);

	vk::ShaderModule createModule(std::string filename, vk::Device device, bool debug);
}
