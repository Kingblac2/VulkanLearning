#pragma once
#include<vulkan/vulkan.hpp>

#include<iostream>
#include<vector>
#include<set>
#include<string>
#include <optional>
#include<fstream>

#include <glm/glm.hpp>
#include <glm/gtc//matrix_transform.hpp>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//------------ASSETS---------------//
enum class meshTypes {
	ZELDA
};

#ifndef _DEBUG
#define _DEBUG 0
#endif // !_DEBUG

std::vector<std::string> split(std::string line, std::string delimiter);
