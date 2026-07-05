#pragma once
#define GLFW_INCLUDE_VULKAN
#include<glfw3.h>
#include "config.h"
#include "frame.h"
#include "scene.h"
#include "triangle_mesh.h"
#include "vertex_menagerie.h"
#include "image.h"
#include"texture.h"
#include "cubemap.h"

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
	std::vector<pipelineTypes> pipelines = { {pipelineTypes::SKY, pipelineTypes::STANDARD} };
	std::unordered_map<pipelineTypes, vk::PipelineLayout> layout;
	std::unordered_map<pipelineTypes, vk::RenderPass> renderpass;
	std::unordered_map<pipelineTypes,vk::Pipeline> pipeline;

	//command-related variables
	vk::CommandPool commandPool;
	vk::CommandBuffer mainCommandBuffer;
	 
	//synchronization-related variables
	int maxFramesInFlight, currentframeNumber;


	//Descriptor objects
	std::unordered_map<pipelineTypes,vk::DescriptorSetLayout> frameSetLayout;
	vk::DescriptorPool frameDescriptorPool;
	std::unordered_map<pipelineTypes,vk::DescriptorSetLayout> meshSetLayout;
	vk::DescriptorPool meshDescriptorPool;

	//asset pointers
	VertexMenagerie* meshes;
	std::unordered_map<meshTypes, vkImage::Texture*> materials;
	vkImage::CubeMap* cubemap;

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
	void record_draw_commands_sky(vk::CommandBuffer& commandBuffer, uint32_t imageIndex,Scene* scene);
	void record_draw_commands_standard(vk::CommandBuffer& commandBuffer, uint32_t imageIndex,Scene* scene);
	void render_objects(vk::CommandBuffer commandBuffer, meshTypes objectType, uint32_t& stratInstance, uint32_t instanceCount);


	void cleanup_swapchian();
};