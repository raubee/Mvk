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
		mvk::GraphicPipeline standard;
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
		//const auto modelPath = "assets/models/adamHead/adamHead1.gltf";
		//const auto modelPath = "assets/models/lantern/lantern.gltf";
		//const auto modelPath = "assets/models/buggy/buggy.gltf";

		models.scene.loadFromFile(&device, transferQueue, modelPath);

		std::array<vk::DescriptorSetLayout, 3> descriptorSetLayouts = {
			mvk::Scene::getDescriptorSetLayout(&device),
			mvk::Model::getDescriptorSetLayout(&device),
			mvk::BaseMaterial::getDescriptorSetLayout(&device)
		};

		const auto descriptorCount =
			static_cast<int32_t>(descriptorSetLayouts.size());

		const auto shaderStageInfo =
			models.scene.materials[0]->getPipelineShaderStageCreateInfo();

		pipelines.standard.build(&device,
		                         swapchain.getSwapchainExtent(),
		                         renderPass.getRenderPass(),
		                         shaderStageInfo,
		                         descriptorSetLayouts.data(),
		                         descriptorCount);
	}

	~GltfViewer()
	{
		models.scene.release();
		pipelines.standard.release();
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

		const auto graphicPipeline = pipelines.standard;
		const auto pipelineLayout = graphicPipeline.getPipelineLayout();
		const auto pipeline = graphicPipeline.getPipeline();

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		                           pipeline);

		for (const auto& node : models.scene.nodes)
		{
			if (!node->hasMesh) continue;

			const auto material =
				dynamic_cast<mvk::BaseMaterial*>(
					models.scene.materials.at(node->matId));

			std::vector<vk::DescriptorSet> descriptorSets = {
				scene.getDescriptorSet(0),
				node->getDescriptorSet(),
				material->getDescriptorSet()
			};

			const auto descriptorCount =
				static_cast<uint32_t>(descriptorSets.size());

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                 pipelineLayout, 0,
			                                 descriptorCount,
			                                 descriptorSets.data(), 0, nullptr);

			vk::Viewport viewport = {
				.x = 0.0f,
				.y = 0.0f,
				.width = static_cast<float>(extent.width),
				.height = static_cast<float>(extent.height),
				.minDepth = 0.0f,
				.maxDepth = 1.0f
			};

			commandBuffer.setViewport(0, viewport);

			vk::Rect2D scissor = {
				.offset = {0, 0},
				.extent = extent
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

		commandBuffer.endRenderPass();
		commandBuffer.end();
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
