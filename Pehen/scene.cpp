#include "scene.h"

Scene::Scene()
{
	positions.insert({ meshTypes::ZELDA,{} });

	positions[meshTypes::ZELDA].push_back(glm::vec3(19.0f, 0.0f, 0.0f));
}
