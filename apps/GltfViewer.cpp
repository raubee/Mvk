#include "AppBase.h"
#include "Model.h"
#include "BaseMaterial.h"

class GltfViewer : public mvk::AppBase
{
	struct Models
	{
		mvk::Model scene;
	}
	models;

	struct GraphicPipelines
	{
		mvk::GraphicPipeline opaque;
		mvk::GraphicPipeline alpha;
	}
	pipelines;

public:
	GltfViewer() : AppBase(mvk::AppInfo{
		.appName = "GltfViewer"
	})
	{
		scene.camera.setPerspective(45.0f, float(width) / float(height),
		                            0.1f, 100.0f);

		//scene.camera.setType(Camera::CameraType::FLY);
		//scene.camera.setPosition(glm::vec3(0.0f, 0.0f, -2.0f));
		scene.camera.setType(Camera::CameraType::ORBIT);
		scene.camera.setLookAt(glm::vec3(0.0f, 0.5f, 0.0f));
		scene.camera.setDistance(1.5f);

		const auto modelPath = "assets/models/flightHelmet/FlightHelmet.gltf";
		//const auto modelPath = "assets/models/scifiHelmet/SciFiHelmet.gltf";
		//const auto modelPath = "assets/models/camera/AntiqueCamera.gltf";
		//const auto modelPath = "assets/models/lantern/lantern.gltf";
		//const auto modelPath = "assets/models/buggy/buggy.gltf";

		models.scene.loadFromFile(&device, transferQueue, modelPath);

		const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			mvk::Scene::getDescriptorSetLayout(&device),
			mvk::Model::getDescriptorSetLayout(&device),
			mvk::BaseMaterial::getDescriptorSetLayout(&device)
		};

		const auto shaderStageInfo =
			models.scene.materials[0]->getPipelineShaderStageCreateInfo();

		const mvk::GraphicPipelineCreateInfo opaquePipelineCreateInfo =
		{
			.extent = swapchain.getSwapchainExtent(),
			.renderPass = renderPass.getRenderPass(),
			.shaderStageCreateInfos = shaderStageInfo,
			.descriptorSetLayouts = descriptorSetLayouts,
			.frontFace = vk::FrontFace::eCounterClockwise
		};

		pipelines.opaque.build(&device, opaquePipelineCreateInfo);

		const mvk::GraphicPipelineCreateInfo alphaPipelineCreateInfo =
		{
			.extent = swapchain.getSwapchainExtent(),
			.renderPass = renderPass.getRenderPass(),
			.shaderStageCreateInfos = shaderStageInfo,
			.descriptorSetLayouts = descriptorSetLayouts,
			.frontFace = vk::FrontFace::eCounterClockwise,
			.alpha = true
		};

		pipelines.alpha.build(&device, alphaPipelineCreateInfo);
	}

	~GltfViewer()
	{
		models.scene.release();
		pipelines.opaque.release();
		pipelines.alpha.release();
	}

	void buildCommandBuffer(const vk::CommandBuffer commandBuffer,
	                        const vk::Framebuffer framebuffer) override
	{
		const auto extent = swapchain.getSwapchainExtent();

		const vk::CommandBufferBeginInfo commandBufferBeginInfo{};
		commandBuffer.begin(commandBufferBeginInfo);

		const std::array<float, 4> clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		std::array<vk::ClearValue, 2> clearValues{};
		clearValues[0].setColor(clearColor);
		clearValues[1].setDepthStencil({1.0f, 0});

		const vk::RenderPassBeginInfo renderPassBeginInfo = {
			.renderPass = renderPass.getRenderPass(),
			.framebuffer = framebuffer,
			.renderArea = {
				.offset = {0, 0},
				.extent = extent
			},
			.clearValueCount = static_cast<uint32_t>(clearValues.size()),
			.pClearValues = clearValues.data()
		};

		commandBuffer.beginRenderPass(renderPassBeginInfo,
		                              vk::SubpassContents::eInline);

		renderPipeline(commandBuffer, pipelines.opaque,
		               mvk::AlphaMode::NO_ALPHA);
		renderPipeline(commandBuffer, pipelines.alpha,
		               mvk::AlphaMode::ALPHA_BLEND);

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	void renderPipeline(const vk::CommandBuffer commandBuffer,
	                    const mvk::GraphicPipeline graphicPipeline,
	                    const mvk::AlphaMode alphaMode)
	{
		const auto pipelineLayout = graphicPipeline.getPipelineLayout();
		const auto pipeline = graphicPipeline.getPipeline();

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		                           pipeline);

		for (const auto& node : models.scene.nodes)
		{
			renderNode(commandBuffer, node, pipelineLayout, alphaMode);
		}
	}

	void renderNode(const vk::CommandBuffer commandBuffer, mvk::Node* node,
	                const vk::PipelineLayout pipelineLayout,
	                const mvk::AlphaMode alphaMode)
	{
		if (!node->hasMesh) return;

		const auto material =
			dynamic_cast<mvk::BaseMaterial*>(
				models.scene.materials.at(node->matId));

		if (material->alphaMode != alphaMode) return;

		std::vector<vk::DescriptorSet> descriptorSets = {
			scene.getDescriptorSet(0),
			node->getDescriptorSet(),
			material->getDescriptorSet()
		};

		const auto descriptorCount =
			static_cast<uint32_t>(descriptorSets.size());

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		                                 pipelineLayout, 0, descriptorCount,
		                                 descriptorSets.data(), 0, nullptr);

		vk::Viewport viewport = {
			.x = 0.0f,
			.y = 0.0f,
			.width = static_cast<float>(width),
			.height = static_cast<float>(height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		commandBuffer.setViewport(0, viewport);

		vk::Rect2D scissor = {
			.offset = {0, 0},
			.extent = swapchain.getSwapchainExtent()
		};

		commandBuffer.setScissor(0, scissor);

		const auto vertexBuffer = models.scene.vertexBuffer;
		const auto indexBuffer = models.scene.indexBuffer;

		vk::DeviceSize offsets[] = {0};

		commandBuffer.
			bindVertexBuffers(0, 1, &vertexBuffer.buffer, offsets);

		if (node->hasIndices)
		{
			commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
			                              vk::IndexType::eUint32);

			commandBuffer.
				drawIndexed(node->indexCount, 1,
				            node->startIndex, 0, 0);
		}
		else
		{
			commandBuffer.draw(node->vertexCount, 1,
			                   node->startVertex, 0);
		}
	}
};

GltfViewer* objViewer;

int main()
{
	objViewer = new GltfViewer();
	objViewer->run();
	delete objViewer;
	return EXIT_SUCCESS;
}
