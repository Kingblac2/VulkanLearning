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


//------------ASSETS---------------//
enum class meshTypes {
	TRIAGLE,
	SQUARE,
	STAR
};

#ifndef _DEBUG
#define _DEBUG 0
#endif // !_DEBUG
