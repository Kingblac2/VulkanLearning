#include "app.h"


App::App(int width, int height, bool debug)
{
	build_glfw_window(width, height, debug);

	graphicsEngine = new Engine(width,height,window,debug);

	scene = new Scene();
}


void App::build_glfw_window(int width, int height, bool debugMode)
{

	//intialize glfw
	glfwInit();

	//no default rendering client, we'll hook vulkan up
	//to the window later
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//resizing breaks the swapchain, we'll disable it for now
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	if (window = glfwCreateWindow(width, height, "ID THIS", nullptr, nullptr))
	{
		if (debugMode)
		{
			std::cout << "Successfully made glfw" << "Width:" << width << "Height" << height;
		}
	}
	else
	{
		if (debugMode)
		{
			std::cout << "Failed to Create Window";
		}
	}
}
void App::run()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		graphicsEngine->render(scene);
		calculateFrameRates();
	}
}

void App::calculateFrameRates()
{
	currentTime = glfwGetTime();
	double delta = currentTime - lastTime;

	if (delta >= 1)
	{
		int framerate{ std::max(1,int(numFrames / delta)) };
		std::stringstream title;
		title << "Running at " << framerate << " fps. ";
		glfwSetWindowTitle(window, title.str().c_str());
		lastTime = currentTime;
		numFrames = -1;
		framerate = float(1000.0 / framerate);
	}

	++numFrames;
}
App::~App()
{
	delete graphicsEngine;
	delete scene;
}