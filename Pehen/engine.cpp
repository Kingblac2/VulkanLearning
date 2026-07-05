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
#include "obj_mesh.h"
#include "texture.h"

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
	bindings.count = 1;
	//binding
	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eUniformBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	frameSetLayout[pipelineTypes::SKY] = vkInit::make_descriptor_set_layout(device, bindings);


	bindings.count = 2;
	
	//binding
	bindings.indices.push_back(1);
	bindings.types.push_back(vk::DescriptorType::eStorageBuffer);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eVertex);

	frameSetLayout[pipelineTypes::STANDARD] = vkInit::make_descriptor_set_layout(device, bindings);


	bindings.counts.clear();
	bindings.indices.clear();
	bindings.stages.clear();
	bindings.types.clear();

	bindings.count = 1;
	
	//binding 2
	bindings.indices.push_back(0);
	bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	bindings.counts.push_back(1);
	bindings.stages.push_back(vk::ShaderStageFlagBits::eFragment);

	

	meshSetLayout[pipelineTypes::SKY] = vkInit::make_descriptor_set_layout(device, bindings);
	meshSetLayout[pipelineTypes::STANDARD] = vkInit::make_descriptor_set_layout(device, bindings);
}

void Engine::make_pipeline()
{
	vkInit::PipelineBuilder pipelineBuilder(device);

	pipelineBuilder.specify_overwrite(false);
	pipelineBuilder.set_overwrite_mode(false);
	pipelineBuilder.specify_vertex_shader("sky_vertex.spv");
	pipelineBuilder.specify_fragment_shader("sky_fragment.spv");
	pipelineBuilder.specify_swapchain_extent(swapchianExtent);
	pipelineBuilder.clear_depth_attachment();
	pipelineBuilder.add_descriptor_set_layout(frameSetLayout[pipelineTypes::SKY]);
	pipelineBuilder.add_descriptor_set_layout(meshSetLayout[pipelineTypes::SKY]);
	pipelineBuilder.add_color_attachment(swapchianFormat, 0);

	vkInit::GraphicsPipelineOutBundle output = pipelineBuilder.build();	
	
	layout[pipelineTypes::SKY] = output.layout;
	renderpass[pipelineTypes::SKY] = output.renderpass;
	pipeline[pipelineTypes::SKY] = output.pipeline;

	pipelineBuilder.reset();

	pipelineBuilder.specify_overwrite(true);
	pipelineBuilder.specify_vertex_format(
		vkMesh::getPosColorBindingDescription(),
		vkMesh::getPosColorAttributeDescriptions()
	);
	pipelineBuilder.specify_vertex_shader("vert.spv");
	pipelineBuilder.specify_fragment_shader("frag.spv");
	pipelineBuilder.specify_swapchain_extent(swapchianExtent);
	pipelineBuilder.specify_depth_attachment(swapchainFrames[0].depthFormat, 1);
	pipelineBuilder.add_descriptor_set_layout(frameSetLayout[pipelineTypes::STANDARD]);
	pipelineBuilder.add_descriptor_set_layout(meshSetLayout[pipelineTypes::STANDARD]);
	pipelineBuilder.add_color_attachment(swapchianFormat, 0);

	output = pipelineBuilder.build();

	layout[pipelineTypes::STANDARD] = output.layout;
	renderpass[pipelineTypes::STANDARD] = output.renderpass;
	pipeline[pipelineTypes::STANDARD] = output.pipeline;
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
	frameDescriptorPool = vkInit::make_descriptor_pool(device, static_cast<uint32_t>(swapchainFrames.size()) * 2, bindings);

	for (vkUtil::SwapChainFrame& frame : swapchainFrames) {
		frame.imageAvailable = vkInit::make_semaphore(device, debugMode);
		frame.renderFinished = vkInit::make_semaphore(device, debugMode);
		frame.inFlight = vkInit::make_fence(device, debugMode);

		frame.make_descriptor_resources();

		frame.descriptorSet[pipelineTypes::SKY] = vkInit::allocate_descriptor_Set(device, frameDescriptorPool, frameSetLayout[pipelineTypes::SKY]);
		frame.descriptorSet[pipelineTypes::STANDARD] = vkInit::allocate_descriptor_Set(device, frameDescriptorPool, frameSetLayout[pipelineTypes::STANDARD]);
		
		frame.record_write_operations();
	}
}

void Engine::make_assets()
{
	meshes = new VertexMenagerie();
	std::unordered_map<meshTypes, std::vector<const char*>> model_filenames = {
		{meshTypes::ZELDA,{"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\models\\zelda export.obj", "D:\\Graphic Programming\\Vulkan Learning\\Pehen\\models\\zelda export.mtl"}}
	};

	glm::mat4 preTransform =
		glm::rotate(glm::mat4(1.0f),
			glm::radians(90.0f),
			glm::vec3(1.0f, 0.0f, 0.0f))   // upright

		* glm::rotate(glm::mat4(1.0f),
			glm::radians(-90.0f),
			glm::vec3(0.0f, 1.0f, 0.0f))   // face camera

		* glm::scale(glm::mat4(1.0f),
			glm::vec3(5.0f));
	for (std::pair<meshTypes, std::vector<const char*>> pair : model_filenames)
	{
		vkMesh::ObjMesh model(pair.second[0], pair.second[1],preTransform);
		meshes->consume(pair.first,model.vertices, model.indices);
	}


	FinializationChunk finalizationChunk;
	finalizationChunk.logicalDevice = device;
	finalizationChunk.physicalDevice = physicalDevice;
	finalizationChunk.queue = graphicsQueue;
	finalizationChunk.commandBuffer = mainCommandBuffer;
	meshes->finalize(finalizationChunk);

	//Materials
	std::unordered_map<meshTypes, std::vector<const char*>> filenames = {
		{meshTypes::ZELDA ,{"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\white.jpg"}},
};

	//Make a descriptor pool
	//...
	vkInit::descriptorSetLayoutData bindings;
	bindings.count = 1;
	bindings.types.push_back(vk::DescriptorType::eCombinedImageSampler);
	meshDescriptorPool = vkInit::make_descriptor_pool(device, static_cast<uint32_t>(filenames.size()) + 1, bindings);

	vkImage::TextureInputChunk textureInfo;
	textureInfo.commandBuffer = mainCommandBuffer;
	textureInfo.queue = graphicsQueue;
	textureInfo.logicalDevice = device;
	textureInfo.physicalDevice = physicalDevice;
	textureInfo.layout = meshSetLayout[pipelineTypes::STANDARD];
	textureInfo.descriptorPool = meshDescriptorPool;

	for (const auto& [object, filename] : filenames) {
		textureInfo.filenames = filename;
		materials[object] = new vkImage::Texture(textureInfo);
	}

	
	textureInfo.layout = meshSetLayout[pipelineTypes::SKY];
	textureInfo.filenames = { {
			"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\front.png",
			"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\back.png",
			"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\left.png",
			"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\right.png",
			"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\down.png",
			"D:\\Graphic Programming\\Vulkan Learning\\Pehen\\tex\\up.png",

	} };

	cubemap = new vkImage::CubeMap(textureInfo);
}
void Engine::prepare_frame(uint32_t imageIndex, Scene* scene)
{

	vkUtil::SwapChainFrame& _frame = swapchainFrames[imageIndex];

	glm::vec4 cam_vec_forwards = { 1.0f, 0.0f, 0.0f,0.0f };
	glm::vec4 cam_vec_right = { 0.0f, -1.0f,0.0f, 0.0f };
	glm::vec4 cam_vec_up = { 0.0f, 0.0f,1.0f,0.0f };



	_frame.cameraVectorData.forwards = cam_vec_forwards;
	_frame.cameraVectorData.right = cam_vec_right;
	_frame.cameraVectorData.up = cam_vec_up;
	memcpy(
		_frame.camerVectorWriteLocation,
		&(_frame.cameraVectorData),
		sizeof(vkUtil::CameraVectors));

	glm::vec3 eye = {0.0f, 0.0f, 1.0f};
	glm::vec3 center = { 1.0f, 0.0f,1.0f };
	glm::vec3 up  = { 0.0f, 0.0f,1.0f };
	glm::mat4 view = glm::lookAt(eye,center,up);


	glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(swapchianExtent.width) / static_cast<float>(swapchianExtent.height) ,
		0.1f, 100.0f); 
	
	projection[1][1] *= -1;

	_frame.cameraMatrixData.view = view;
	_frame.cameraMatrixData.projection = projection;
	_frame.cameraMatrixData.viewProjection = projection * view;
	memcpy(
		_frame.camerMatrixWriteLocation, 
		&(_frame.cameraMatrixData),
		sizeof(vkUtil::CameraMatrices));

	

	size_t i = 0;
	for (std::pair<meshTypes, std::vector<glm::vec3>> pair : scene->positions) {
		for (glm::vec3& postion : pair.second)
		{
			_frame.modelTransforms[i++] = glm::translate(glm::mat4(1.0f), postion);
		}
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

void Engine::record_draw_commands_sky(vk::CommandBuffer& commandBuffer, uint32_t imageIndex, Scene* scene)
{

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderpass[pipelineTypes::SKY];
	renderPassInfo.framebuffer = swapchainFrames[imageIndex].framebuffer[pipelineTypes::SKY];
	renderPassInfo.renderArea.offset.x = 0;
	renderPassInfo.renderArea.offset.y = 0;
	renderPassInfo.renderArea.extent = swapchianExtent;
	vk::ClearValue clearcolor = { std::array<float , 4>{1.0f,0.5f,0.25f,1.0f} };
	

	std::vector<vk::ClearValue> clearValues = { {clearcolor} };

	renderPassInfo.clearValueCount = clearValues.size();
	renderPassInfo.pClearValues = clearValues.data();

	commandBuffer.beginRenderPass(&renderPassInfo, vk::SubpassContents::eInline);

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline[pipelineTypes::SKY]);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		layout[pipelineTypes::SKY], 0,
		swapchainFrames[imageIndex].descriptorSet[pipelineTypes::SKY], nullptr);



	cubemap->use(commandBuffer, layout[pipelineTypes::SKY]);

	commandBuffer.draw(6, 1, 0, 0);

	commandBuffer.endRenderPass();





}

void Engine::record_draw_commands_standard(vk::CommandBuffer& commandBuffer, uint32_t imageIndex, Scene* scene)
{

	vk::RenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.renderPass = renderpass[pipelineTypes::STANDARD];
	renderPassInfo.framebuffer = swapchainFrames[imageIndex].framebuffer[pipelineTypes::STANDARD];
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

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline[pipelineTypes::STANDARD]);

	commandBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		layout[pipelineTypes::STANDARD], 0, 
		swapchainFrames[imageIndex].descriptorSet[pipelineTypes::STANDARD], nullptr);

	prepare_scene(commandBuffer);

	uint32_t startInstance = 0;
	for (std::pair<meshTypes, std::vector<glm::vec3>> pair : scene->positions) {
		render_objects(
			commandBuffer, pair.first, startInstance, static_cast<uint32_t>(pair.second.size())
		);
	}



	commandBuffer.endRenderPass();


}

void Engine::render_objects(vk::CommandBuffer commandBuffer, meshTypes objectType, uint32_t& startInstance, uint32_t instanceCount)
{
	int  indexCount = meshes->IndexCounts.find(objectType)->second;
	int firstIndex = meshes->firstIndices.find(objectType)->second;
	materials[objectType]->use(commandBuffer, layout[pipelineTypes::STANDARD]);
	commandBuffer.drawIndexed(indexCount, instanceCount, firstIndex,0, startInstance);
	startInstance += instanceCount;
}

void Engine::render(Scene* scene)
{
	vk::Result v1 =	device.waitForFences(1, &swapchainFrames[currentframeNumber].inFlight, VK_TRUE, UINT64_MAX);
	vk::Result v2 = device.resetFences(1, &(swapchainFrames[currentframeNumber].inFlight));
	
	if (v1 != vk::Result::eSuccess && v2 != vk::Result::eSuccess)
		__halt();

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
	


	vk::CommandBuffer& commandBuffer = swapchainFrames[currentframeNumber].commandBuffer;

	commandBuffer.reset();

	prepare_frame(imageIndex,scene);


	vk::CommandBufferBeginInfo beginInfo = {};

	try {
		commandBuffer.begin(beginInfo);

	}
	catch (vk::SystemError err) {
		if (debugMode) {
			std::cout << "Failed to begin recording command buffer" << std::endl;
		}
	}

	record_draw_commands_sky(commandBuffer, imageIndex, scene);
	record_draw_commands_standard(commandBuffer, imageIndex, scene);

	try {
		commandBuffer.end();
	}
	catch (vk::SystemError err) {
		if (debugMode)
		{
			std::cout << " Failed to Finish Recording command buffer" << std::endl;
		}
	}

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

	for (pipelineTypes pipeline_type : pipelines)
	{
		device.destroyPipeline(pipeline	   [pipeline_type]);
		device.destroyPipelineLayout(layout[pipeline_type]);
		device.destroyRenderPass(renderpass[pipeline_type]);
	}
	


	cleanup_swapchian();

	for (pipelineTypes pipeline_type : pipelines) {
		device.destroyDescriptorSetLayout(frameSetLayout[pipeline_type]);
	device.destroyDescriptorSetLayout(meshSetLayout[pipeline_type]);
	}
	delete meshes;

	for (const auto& [key,texture] : materials)
	{
		delete texture;
	}

	delete cubemap;

	device.destroyDescriptorPool(meshDescriptorPool);

	device.destroy();

	instance.destroySurfaceKHR(surface);
	instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
	instance.destroy();

	glfwTerminate();
}