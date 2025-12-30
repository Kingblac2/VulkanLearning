#include "engine.h"
#include "instance.h"
#include "logging.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"
#include "framebuffer.h"
#include "commands.h"
#include "sync.h"
#include "descriptor.h"

Engine::Engine(int width, int height, GLFWwindow* window, bool debugMode) : width(width),height(height),window(window),debugMode(debugMode)
{
	if (debugMode)
	{
		std::cout << "Naking a graphics engine\n";
	}

	make_instance();

	make_device();

	make_descriptor_set_layouts();

	make_pipeline();

	finalize_setup();

	make_assets();
}


void Engine::make_instance()
{
	instance = vkInit::make_instance(debugMode, "ID THIS");
	dldi = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr); 

	if (debugMode) {
		debugMessenger = vkInit::make_debug_messenger(instance, dldi);
	}
	VkSurfaceKHR c_style_surface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &c_style_surface) != VK_SUCCESS)
	{
		if (debugMode) {
			std::cout << "Failed to abstrtact the glfw surface for vulkan.\n";
		}
	}
	else if (debugMode)
	{
		std::cout << "Succesfully abstracted the glfw surface for VUlKAN.\n";
	}
	surface = c_style_surface;
}

void Engine::make_device()
{
	physicalDevice = vkInit::choose_physical_device(instance, debugMode);
	device = vkInit::create_logical_device(physicalDevice,surface,debugMode);
	std::array<vk::Queue,2> queues = vkInit::get_queue(physicalDevice, device,surface, debugMode);
	graphicsQueue = queues[0];
	presentQueue = queues[1];
	make_swapchain();
	currentframeNumber = 0;
}

void Engine::make_swapchain()
{
	vkInit::SwapChainBundle bundle = vkInit::create_swapchain(device, physicalDevice, surface, width, height, debugMode);
	swapchain = bundle.swapchain;
	swapchainFrames = bundle.frames;
	swapchianFormat = bundle.format;
	swapchianExtent = bundle.extent;
	maxFramesInFlight = static_cast<int>(swapchainFrames.size());

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {

		frame.logicalDevice = device;
		frame.physicalDevice = physicalDevice;
		frame.width = swapchianExtent.width;
		frame.height = swapchianExtent.height;

		frame.make_depth_resources();
	}

}

void Engine::recreate_swapchain() {

	width = 0;
	height = 0;
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	device.waitIdle();

	cleanup_swapchian();
	make_swapchain();
	make_framebuffers();
	make_frame_resource();
	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	vkInit::make_frame_command_buffers(commandBufferInput, debugMode);


}

void Engine::make_descriptor_set_layouts()
{
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 2;
	//binding
	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);
	
	//binding
	bindings.indices.push_back(1);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	frameSetLayout = vkInit::make_descriptor_set_layout(device, bindings);


	bindings.count = 1;
	
	//binding 2
	bindings.indices[0] = (0);
	bindings.types[0]   = (vk::DescriptorType::eCombinedImageSampler);
	bindings.counts[0]  = (1);
	bindings.stages[0]  = (vk::ShaderStageFlagBits::eFragment);

	

	meshSetLayout = vkInit::make_descriptor_set_layout(device, bindings);
}

void Engine::make_pipeline()
{
	vkInit::GraphicsPipelineBundle specification = {};
	specification.device = device;
	specification.vertexFilepath = "vert.spv";
	specification.fragmentFilepath = "frag.spv";
	specification.swapChainExtent = swapchianExtent;
	specification.SwapchainImageFormat = swapchianFormat;
	specification.depthFormat = swapchainFrames[0].depthFormat;
	specification.descriptorSetLayouts = { frameSetLayout, meshSetLayout };

	vkInit::GraphicsPipelineOutBundle output = vkInit::make_graphics_pipeline(specification,debugMode);
	layout = output.layout;
	renderpass = output.renderpass;
	pipeline = output.pipeline;
}

void Engine::make_framebuffers() {
	vkInit::framebufferInput frameBufferInput;
	frameBufferInput.device = device;
	frameBufferInput.renderpass = renderpass;
	frameBufferInput.swapchainExtent = swapchianExtent;
	vkInit::make_framebuffers(frameBufferInput, swapchainFrames, debugMode);

}

void Engine::make_frame_resource () {

	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 2;
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	frameDescriptorPool = vkInit::make_descriptor_pool(device, static_cast<uint32_t>(swapchainFrames.size()), bindings);

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {
		frame.imageAvailable = vkInit::make_semaphore(device, debugMode);
		frame.renderFinished = vkInit::make_semaphore(device, debugMode);
		frame.inFlight = vkInit::make_fence(device, debugMode);

		frame.make_descriptor_resources();

		frame.descriptorSet = vkInit::allocate_descriptor_Set(device, frameDescriptorPool, frameSetLayout);
	}
}

void Engine::make_assets()
{
	meshes = new VertexMenagerie();

	
	std::vector<float> vertices = { {
		 0.0f, -0.1f, 1.0f, 1.0f, 1.0f, 0.5f, 0.0f, // 0
		 0.1f,  0.1f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, // 1
		-0.1f,  0.1f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f //  2
	} };

	std::vector<uint32_t> indices = { {
			0,1,2
	} };	

	meshTypes type = meshTypes::TRIAGLE;
	meshes->consume(type, vertices, indices);

	vertices = { {
		-0.1f,  0.1f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // 0
		-0.1f, -0.1f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, // 1
		 0.1f, -0.1f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, // 2
		 0.1f,  0.1f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f	// 3
	} };

	indices = { {
		0,1,2,
		2,3,0
	} };	

	type = meshTypes::SQUARE;
	meshes->consume(type, vertices, indices);

	vertices = { {
			 -0.1f, -0.05f, 1.0f, 1.0f, 1.0f, 0.0f, 0.25f, //0
			-0.04f, -0.05f, 1.0f, 1.0f, 1.0f, 0.3f, 0.25f, //1
			-0.06f,   0.0f, 1.0f, 1.0f, 1.0f, 0.2f,  0.5f, //2
			  0.0f,  -0.1f, 1.0f, 1.0f, 1.0f, 0.5f,  0.0f, //3
			 0.04f, -0.05f, 1.0f, 1.0f, 1.0f, 0.7f, 0.25f, //4
			  0.1f, -0.05f, 1.0f, 1.0f, 1.0f, 1.0f, 0.25f, //5
			 0.06f,   0.0f, 1.0f, 1.0f, 1.0f, 0.8f,  0.5f, //6
			 0.08f,   0.1f, 1.0f, 1.0f, 1.0f, 0.9f,  1.0f, //7
			  0.0f,  0.02f, 1.0f, 1.0f, 1.0f, 0.5f,  0.6f, //8
			-0.08f,   0.1f, 1.0f, 1.0f, 1.0f, 0.1f,  1.0f  //9
		} };

	indices = { {
				0, 1, 2,
				1, 3, 4,
				2, 1, 4,
				4, 5, 6,
				2, 4, 6,
				6, 7, 8,
				2, 6, 8,
				2, 8, 9
		} };
	type = meshTypes::STAR;
	meshes->consume(type, vertices, indices);

	FinializationChunk finalizationChunk;
	finalizationChunk.logicalDevice = device;
	finalizationChunk.physicalDevice = physicalDevice;
	finalizationChunk.queue = graphicsQueue;
	finalizationChunk.commandBuffer = mainCommandBuffer;
	meshes->finalize(finalizationChunk);

	//Materials
	std::unordered_map<meshTypes, const char*> filenames = {
		{meshTypes::TRIAGLE ,"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\Mitaka.jpg"},
		{meshTypes::SQUARE ,"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\Makima.jpg"},
		{meshTypes::STAR  , "D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\Elf.jpg" }
	};

	//Make a descriptor pool
	//...
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 1;
	bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	meshDescriptorPool = vkInit::make_descriptor_pool(device, static_cast<uint32_t>(filenames.size()), bindings);

	vkImage::TextureInputChunk textureInfo;
	textureInfo.commandBuffer = mainCommandBuffer;
	textureInfo.queue = graphicsQueue;
	textureInfo.logicalDevice = device;
	textureInfo.physicalDevice = physicalDevice;
	textureInfo.layout = meshSetLayout;
	textureInfo.descriptorPool = meshDescriptorPool;

	for (const auto& [object, filename] : filenames) {
		textureInfo.filename = filename;
		materials[object] = new vkImage::Texture(textureInfo);
	}
}
void Engine::prepare_frame(uint32_t imageIndex, Scene* scene)
{

	vkUtil::SwapChainFrame _frame = swapchainFrames[imageIndex];

	glm::vec3 eye = {1.0f, 0.0f, -1.0f};
	glm::vec3 center = { 0.0f, 0.0f,0.0f };
	glm::vec3 up  = { 0.0f, 0.0f,-1.0f };
	glm::mat4 view = glm::lookAt(eye,center,up);


	glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchianExtent.width) / static_cast<float>(swapchianExtent.height) ,
		0.1f, 10.0f); 
	
	projection[1][1] *= -1;

	_frame.cameraData.view = view;
	_frame.cameraData.projection = projection;
	_frame.cameraData.viewProjection = projection * view;
	memcpy(
		_frame.camerDataWriteLocation, 
		&(_frame.cameraData),
		sizeof(vkUtil::UBO));


	size_t i = 0;
	for (glm::vec3& postion : scene->trianglePositions)
	{
		_frame.modelTransforms[i++] = glm::translate(glm::mat4(1.0f), postion);
	}
	for (glm::vec3& postion : scene->squarePositions)
	{
		_frame.modelTransforms[i++] = glm::translate(glm::mat4(1.0f), postion);
	}
	for (glm::vec3& postion : scene->starPositions)
	{
		_frame.modelTransforms[i++] = glm::translate(glm::mat4(1.0f), postion);
	}

	memcpy(_frame.modelBufferWriteLocation, _frame.modelTransforms.data(), i * sizeof(glm::mat4));

	_frame.write_descriptor_set();
}
void Engine::prepare_scene(vk::CommandBuffer commandBuffer)
{
	vk::Buffer vertexBuffers[] = { meshes->vertexBuffer.buffer };
	vk::DeviceSize offsets[] = { 0 };
	commandBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
	commandBuffer.bindIndexBuffer(meshes->indexBuffer.buffer, 0, vk::IndexType::eUint32);
}

void Engine::finalize_setup() {

	make_framebuffers();

	commandPool = vkInit::make_command_pool(device, physicalDevice, surface, debugMode);

	vkInit::commandBufferInputChunk commandBufferInput = { device, commandPool, swapchainFrames };
	mainCommandBuffer = vkInit::make_command_buffer(commandBufferInput, debugMode);
	vkInit::make_frame_command_buffers(commandBufferInput, debugMode);

	make_frame_resource();
}

void Engine::record_draw_commands(vk::CommandBuffer commandBuffer, uint32_t imageIndex, Scene* scene)
{
	vk::CommandBufferBeginInfo beginInfo = {};

	try {
		commandBuffer.begin(beginInfo);

	}
	catch (vk::SystemError err) {
		if (debugMode) {
			std::cout << "Failed to begin recording command buffer" << std::endl;
		}
	}

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderpass;
	renderPassInfo.framebuffer = swapchainFrames[imageIndex].framebuffer;
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapchianExtent;
	vk::ClearValue clearcolor = { std::array<float , 4>{1.0f,0.5f,0.25f,1.0f} };
	vk::ClearValue clearDepth;
	clearDepth.depthStencil = vk::ClearDepthStencilValue({ 1.0f,0 });
	std::vector<vk::ClearValue> clearValues = { {clearcolor,clearDepth} };

	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		layout, 0, swapchainFrames[imageIndex].descriptorSet, nullptr);

	prepare_scene(commandBuffer);

	uint32_t startInstance = 0;
		render_objects(
		commandBuffer, meshTypes::TRIAGLE, startInstance, static_cast<uint32_t>(scene->trianglePositions.size())
	);

	//Squares
	render_objects(
		commandBuffer, meshTypes::SQUARE, startInstance, static_cast<uint32_t>(scene->squarePositions.size())
	);

	//Stars
	render_objects(
		commandBuffer, meshTypes::STAR, startInstance, static_cast<uint32_t>(scene->starPositions.size())
	);


	commandBuffer.endRenderPass();


	// 6. End command buffer


	try {
		commandBuffer.end();
	}
	catch(vk::SystemError err){
		if (debugMode)
		{
			std::cout << " Failed to Finish Recording command buffer" << std::endl;
		}
	}
}

void Engine::render_objects(vk::CommandBuffer commandBuffer, meshTypes objectType, uint32_t& startInstance, uint32_t instanceCount)
{
	int  indexCount = meshes->IndexCounts.find(objectType)->second;
	int firstIndex = meshes->firstIndices.find(objectType)->second;
	materials[objectType]->use(commandBuffer, layout);
	commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex,0, startInstance);
	startInstance += instanceCount;
}

void Engine::render(Scene* scene)
{
	device.waitForFences(1, &swapchainFrames[currentframeNumber].inFlight, VK_TRUE, UINT64_MAX);
	device.resetFences(1, &(swapchainFrames[currentframeNumber].inFlight));
	
	uint32_t imageIndex;
	try {
		vk::ResultValue acquire = device.acquireNextImageKHR(swapchain, UINT64_MAX, swapchainFrames[currentframeNumber].imageAvailable, nullptr);
		 imageIndex = acquire.value ;
	}
	catch (vk::OutOfDateKHRError error) {
		std::cout << "Recreate" << std::endl;
		recreate_swapchain();
		return;
	}
	catch (vk::IncompatibleDisplayKHRError error) {
		std::cout << "Recreate" << std::endl;
		recreate_swapchain();
		return;
	}
	catch (vk::SystemError error) {
		std::cout << "Failed to acquire swapchain image!" << std::endl;
	}
	


	vk::CommandBuffer commandBuffer = swapchainFrames[currentframeNumber].commandBuffer;

	commandBuffer.reset();

	prepare_frame(imageIndex,scene);

	record_draw_commands(commandBuffer, imageIndex, scene);

	vk::SubmitInfo submitInfo = {};

	vk::Semaphore waitSemaphore[] = { swapchainFrames[currentframeNumber].imageAvailable };
	vk::PipelineStageFlags waitStage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphore;
	submitInfo.pWaitDstStageMask = waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vk::Semaphore signalSemaphore[] = { swapchainFrames[currentframeNumber].renderFinished };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphore;

//	device.resetFences(1, &swapchainFrames[currentframeNumber].inFlight);
	try {
		graphicsQueue.submit(submitInfo, swapchainFrames[currentframeNumber].inFlight);
	}
	catch (vk::SystemError err)
	{
		if (debugMode)
		{
			std::cout << "Failed to submit draw sommand buffer" << std::endl;
		}
	}
	
	




	vk::PresentInfoKHR presentInfo = {};
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphore;
	vk::SwapchainKHR swapchains[] = { swapchain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapchains;
	presentInfo.pImageIndices = &imageIndex;

	vk::Result present;
	try {
		present = presentQueue.presentKHR(presentInfo);
	}
	catch (vk::OutOfDateKHRError error) {
		present = vk::Result::eErrorOutOfDateKHR;
	}

	if (present == vk::Result::eErrorOutOfDateKHR || present == vk::Result::eSuboptimalKHR) {
		recreate_swapchain();
		return;
	}
	currentframeNumber = (currentframeNumber + 1) % maxFramesInFlight;

}

void Engine::cleanup_swapchian()
{
	for (vkUtil::SwapChainFrame& frame : swapchainFrames)
	{
		frame.destroy();
	}

	device.destroySwapchainKHR(swapchain);

	device.destroyDescriptorPool(frameDescriptorPool);

}
Engine::~Engine()
{
	device.waitIdle();

	if (debugMode)
	{
		std::cout << "Goodbye Window\n";
	}

	device.destroyCommandPool(commandPool);

	device.destroyPipeline(pipeline);
	device.destroyPipelineLayout(layout);
	device.destroyRenderPass(renderpass);

	cleanup_swapchian();

	device.destroyDescriptorSetLayout(frameSetLayout);

	delete meshes;

	for (const auto& [key,texture] : materials)
	{
		delete texture;
	}
	device.destroyDescriptorSetLayout(meshSetLayout);
	device.destroyDescriptorPool(meshDescriptorPool);

	device.destroy();

	instance.destroySurfaceKHR(surface);
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
	instance.destroy();

	glfwTerminate();
}