#pragma once
#include "config.h"
#include"shaders.h"
#include "render_structs.h"
#include"mesh.h"

namespace vkInit {

	struct GraphicsPipelineBundle {
		vk::Device device;
		std::string vertexFilepath;
		std::string fragmentFilepath;
		vk::Extent2D swapChainExtent;
		vk::Format SwapchainImageFormat, depthFormat;
		std::vector<vk::DescriptorSetLayout> descriptorSetLayouts;
	};
	struct GraphicsPipelineOutBundle {
		vk::PipelineLayout layout;
		vk::RenderPass renderpass;
		vk::Pipeline pipeline;
	};

	vk::PipelineLayout  make_pipeline_layout(vk::Device device, bool debug,std::vector<vk::DescriptorSetLayout> layouts){
		vk::PipelineLayoutCreateInfo layoutInfo;
		layoutInfo.flags = vk::PipelineLayoutCreateFlags();

		layoutInfo.setLayoutCount = static_cast<uint32_t>(layouts.size());
		layoutInfo.pSetLayouts = layouts.data();

		layoutInfo.pushConstantRangeCount = 0;
		
		{
			/* Push Constant No Longer in use
			vk::PushConstantRange pushConstantInfo;
			pushConstantInfo.offset = 0;
			pushConstantInfo.size = sizeof(vkUtil::ObjectData);
			pushConstantInfo.stageFlags = vk::ShaderStageFlagBits::eVertex;
		*/}

		try {
			return device.createPipelineLayout(layoutInfo);
		}
		catch (vk::SystemError err) {
			if (debug)
			{
				std::cout << " Failed to create pipeline layout!" << std::endl;
			}
		}
	}

	vk::AttachmentDescription make_depth_attachment(vk::Format depthFormat)
	{
		vk::AttachmentDescription depthAttachment = {};
		depthAttachment.flags = vk::AttachmentDescriptionFlags();
		depthAttachment.format = depthFormat;
		depthAttachment.samples = vk::SampleCountFlagBits::e1;
		depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;

		// When we write a pixel do we store that pixel yes we do store the pixel
		depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
		depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		return depthAttachment;
	}


	vk::AttachmentReference make_depth_attachment_refrence()
	{
		vk::AttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

		return depthAttachmentRef;
	}
	
	vk::AttachmentDescription make_color_attachment(vk::Format format)
	{
		vk::AttachmentDescription colorAttachment = {};
		colorAttachment.flags = vk::AttachmentDescriptionFlags();
		colorAttachment.format = format;
		colorAttachment.samples = vk::SampleCountFlagBits::e1;
		colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;

		// When we write a pixel do we store that pixel yes we do store the pixel
		colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
		colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
		colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

		return colorAttachment;
	}


	vk::AttachmentReference make_color_attachment_refrence()
	{
		vk::AttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

		return colorAttachmentRef;
	}

	vk::SubpassDescription make_subpass(std::vector<vk::AttachmentReference>& attachments)
	{

		vk::SubpassDescription subpass = {};
		subpass.flags = vk::SubpassDescriptionFlags();
		subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &attachments[0];
		subpass.pDepthStencilAttachment = &attachments[1];

		return subpass;
	}

	vk::RenderPassCreateInfo make_renderpass_info(std::vector<vk::AttachmentDescription>& attachments, vk::SubpassDescription subpass)
	{
		vk::RenderPassCreateInfo renderpassInfo = {};
		renderpassInfo.flags = vk::RenderPassCreateFlags();
		renderpassInfo.attachmentCount = attachments.size();
		renderpassInfo.pAttachments = attachments.data();
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subpass;

		return renderpassInfo;
	}

	vk::RenderPass make_renderpass(vk::Device device, vk::Format swapchainImageFormat, bool debug, vk::Format depthFormat)
	{

		std::vector<vk::AttachmentDescription> attachments;
		std::vector<vk::AttachmentReference> attachmentReferences;

		//color
		attachments.push_back( make_color_attachment(swapchainImageFormat));
	    attachmentReferences.push_back( make_color_attachment_refrence());
		

		//depth
		attachments.push_back(make_depth_attachment(depthFormat));
		attachmentReferences.push_back(make_depth_attachment_refrence());



		vk::SubpassDescription subpass = make_subpass(attachmentReferences);


		vk::RenderPassCreateInfo renderpassInfo = make_renderpass_info(attachments, subpass);

		try {
			return device.createRenderPass(renderpassInfo);
		}
		catch (vk::SystemError err) {
			if (debug) {
				std::cout << "Failed to create renderpass!" << std::endl;
			}
		}
	}
	GraphicsPipelineOutBundle make_graphics_pipeline(GraphicsPipelineBundle specification, bool debug)
	{
		vk::GraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.flags = vk::PipelineCreateFlags();

		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;

		//Vertex Input
		vk::VertexInputBindingDescription bindingDescription = vkMesh::getPosColorBindingDescription();
		std::array<vk::VertexInputAttributeDescription, 3> attributeDescription = vkMesh::getPosColorAttributeDescriptions();
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.flags = vk::PipelineVertexInputStateCreateFlags();
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;

		//Input Assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
		inputAssemblyInfo.flags = vk::PipelineInputAssemblyStateCreateFlags();
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;

		//vertex Shader
		if (debug) {
			std::cout << "Create vertex shader module" << std::endl;
		}

		vk::ShaderModule vertexShader = vkUtil::createModule(specification.vertexFilepath, specification.device, debug);
		vk::PipelineShaderStageCreateInfo vertexShaderInfo = {};
		vertexShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		vertexShaderInfo.stage = vk::ShaderStageFlagBits::eVertex;
		vertexShaderInfo.module = vertexShader;
		// entry-point
		vertexShaderInfo.pName = "main";
		shaderStages.push_back(vertexShaderInfo);

		vk::Viewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = specification.swapChainExtent.width;
		viewport.height = specification.swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f; 

		vk::Rect2D scissor = {};
		scissor.offset.x = 0.0f;
		scissor.offset.y = 0.0f;
		scissor.extent = specification.swapChainExtent;
		vk::PipelineViewportStateCreateInfo viewportState = {};
		viewportState.flags = vk::PipelineViewportStateCreateFlags();
		viewportState.viewportCount = 1; // Daksh New Changes
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;
		pipelineInfo.pViewportState = &viewportState;

		//Rasterizer
		vk::PipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.flags = vk::PipelineRasterizationStateCreateFlags();
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = vk::PolygonMode::eFill;
		rasterizer.cullMode = vk::CullModeFlagBits::eBack;
		rasterizer.frontFace = vk::FrontFace::eClockwise;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.lineWidth = 1.0f; // Daksh New Changes
		pipelineInfo.pRasterizationState = &rasterizer;

		vk::ShaderModule fragmentShader = vkUtil::createModule(specification.fragmentFilepath, specification.device, debug);
		vk::PipelineShaderStageCreateInfo fragmentShaderInfo = {};
		fragmentShaderInfo.flags = vk::PipelineShaderStageCreateFlags();
		fragmentShaderInfo.stage = vk::ShaderStageFlagBits::eFragment;
		fragmentShaderInfo.module = fragmentShader;
		// entry-point
		fragmentShaderInfo.pName = "main";
		shaderStages.push_back(fragmentShaderInfo);
		pipelineInfo.stageCount = shaderStages.size();
		pipelineInfo.pStages = shaderStages.data();

		//Depth-Stencil
		vk::PipelineDepthStencilStateCreateInfo depthState;
		depthState.flags = vk::PipelineDepthStencilStateCreateFlags();
		depthState.depthTestEnable = true;
		depthState.depthWriteEnable = true;
		depthState.depthCompareOp = vk::CompareOp::eLess;
		depthState.depthBoundsTestEnable = false;
		depthState.stencilTestEnable = false;
		pipelineInfo.pDepthStencilState = &depthState;

		//MultiSampeling
		vk::PipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.flags = vk::PipelineMultisampleStateCreateFlags();
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
		pipelineInfo.pMultisampleState = &multisampling;

		// Color Blend
		vk::PipelineColorBlendAttachmentState coloBlendAttachment = {};
		coloBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
		coloBlendAttachment.blendEnable = VK_FALSE;
		vk::PipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.flags = vk::PipelineColorBlendStateCreateFlags();
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = vk::LogicOp::eCopy;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &coloBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;
		pipelineInfo.pColorBlendState = &colorBlending;

		//Pipeline Layout
		if (debug) {
			std::cout << "Create Pipeline Layout " << std::endl;

		}
		vk::PipelineLayout layout = make_pipeline_layout(specification.device, debug,specification.descriptorSetLayouts);
		pipelineInfo.layout = layout;
		
		//Render Pass
		if (debug) {
			std::cout << "Create Render Pass " << std::endl;
		}
		vk::RenderPass renderpass = make_renderpass(specification.device, specification.SwapchainImageFormat, debug,specification.depthFormat);
		pipelineInfo.renderPass = renderpass;

		//Extra stuff
		pipelineInfo.basePipelineHandle = nullptr;

		
		inputAssemblyInfo.pNext = nullptr;
		vertexInputInfo.pNext = nullptr;
		viewportState.pNext = nullptr;
		rasterizer.pNext = nullptr;
		depthState.pNext = nullptr;
		multisampling.pNext = nullptr;
		colorBlending.pNext = nullptr;
		pipelineInfo.pNext = nullptr;

		//Make the pipeline
		if (debug) {
			std::cout << "Create Graphics Pipeline" << std::endl;

		}

		vk::Pipeline graphicsPipeline;
		try {
			graphicsPipeline = (specification.device.createGraphicsPipeline(nullptr, pipelineInfo)).value;
		}
		catch (vk::SystemError err)
		{
			if (debug)
			{
				std::cout << "Failed to create Graphics Pipeline!" << std::endl;
			}
		}
		GraphicsPipelineOutBundle outptr = {};
		outptr.layout = layout;
		outptr.renderpass = renderpass;
		outptr.pipeline = graphicsPipeline;

		specification.device.destroyShaderModule(vertexShader);
		specification.device.destroyShaderModule(fragmentShader);
		return outptr;
	}
}