#pragma once
#include "config.h"
#include <unordered_map>

class Scene {

public:
	Scene();

	std::unordered_map<meshTypes, std::vector<glm::vec3>> positions;
};