#pragma once
#define GLFW_INCLUDE_VULKAN
#include<glfw3.h>
#include "config.h"
#include "frame.h"
#include "scene.h"
#include "triangle_mesh.h"
#include "vertex_menagerie.h"
#include "image.h"

class Engine {

public:
	Engine(int width,int height, GLFWwindow* window,bool debugMode);


	~Engine();

	void render(Scene* scene);

private:

	//whether to print debug messagesin function
	bool debugMode = true;

	//glfw window parameters
	int width{ 640 };
	int height{ 480 };
	GLFWwindow* window{ nullptr };

	//vulkan-instance relate varibles
	vk::Instance instance{ nullptr };
	vk::DebugUtilsMessengerEXT debugMessenger{ nullptr };
	vk::DispatchLoaderDynamic dldi;
	vk::SurfaceKHR surface;

	//devic-realated variables
	vk::PhysicalDevice physicalDevice{ nullptr };
	vk::Device device{ nullptr };
	vk::Queue  graphicsQueue{ nullptr };
	vk::Queue  presentQueue{ nullptr };
	vk::SwapchainKHR swapchain;
	std::vector<vkUtil::SwapChainFrame> swapchainFrames{};
	vk::Format swapchianFormat;
	vk::Extent2D swapchianExtent;

	//pipeline related variables
	vk::PipelineLayout layout;
	vk::RenderPass renderpass;
	vk::Pipeline pipeline;

	//command-related variables
	vk::CommandPool commandPool;
	vk::CommandBuffer mainCommandBuffer;
	 
	//synchronization-related variables
	int maxFramesInFlight, currentframeNumber;


	//Descriptor objects
	vk::DescriptorSetLayout frameSetLayout;
	vk::DescriptorPool frameDescriptorPool;
	vk::DescriptorSetLayout meshSetLayout;	
	vk::DescriptorPool meshDescriptorPool;

	//asset pointers
	VertexMenagerie* meshes;
	std::unordered_map<meshTypes, vkImage::Texture*> materials;

	void make_instance();

	
	//device setup 
	void make_device();
	void make_swapchain();
	void recreate_swapchain();

	//pipline setup
	void make_descriptor_set_layouts();
	void make_pipeline();

	void finalize_setup();
	void make_framebuffers();
	void make_frame_resource();

	void make_assets();


	void prepare_scene(vk::CommandBuffer commandBuffer);
	void prepare_frame(uint32_t imageIndex, Scene* scene);
	void record_draw_commands(vk::CommandBuffer commandBuffer, uint32_t imageIndex,Scene* scene);
	void render_objects(vk::CommandBuffer commandBuffer, meshTypes objectType, uint32_t& stratInstance, uint32_t instanceCount);


	void cleanup_swapchian();
};