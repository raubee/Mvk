#include "AppBase.h"
#include "Model.h"
#include "Texture2D.h"
#include "BaseMaterial.h"
#include "GraphicPipeline.h"

class ObjViewer : public mvk::AppBase
{
	struct Textures
	{
		mvk::Texture2D albedo;
	}
	textures;

	struct Materials
	{
		mvk::BaseMaterial standard;
	}
	materials;

	struct Models
	{
		mvk::Model ganesh;
	}
	models;

	struct GraphicPipelines
	{
		mvk::GraphicPipeline standard;
	}
	pipelines;

public:
	ObjViewer(): AppBase(mvk::AppInfo{
		.appName = "ObjViewer"
	})
	{
		scene.camera.setPerspective(45.0f, float(width) / float(height),
		                            0.1f, 100.0f);

		/*scene.camera.setType(Camera::CameraType::FLY);
		scene.camera.setPosition(glm::vec3(0.0f, 0.0f, -2.0f));*/
		scene.camera.setType(Camera::CameraType::ORBIT);
		scene.camera.setLookAt(glm::vec3(0.0f, 0.5f, 0.0f));
		scene.camera.setDistance(1.5f);

		const auto modelPath = "assets/models/ganesha/ganesha.obj";

		models.ganesh.loadFromFile(&device, transferQueue, modelPath);

		const auto albedoPath =
			"assets/models/ganesha/textures/Ganesha_BaseColor.jpg";

		textures.albedo.loadFromFile(&device, transferQueue, albedoPath,
		                             vk::Format::eR8G8B8A8Srgb);

		mvk::BaseMaterialDescription description;
		description.albedo = &textures.albedo;

		materials.standard.load(&device, description);

		std::array<vk::DescriptorSetLayout, 3> descriptorSetLayouts = {
			mvk::Scene::getDescriptorSetLayout(&device),
			mvk::Model::getDescriptorSetLayout(&device),
			mvk::BaseMaterial::getDescriptorSetLayout(&device)
		};

		const auto descriptorCount =
			static_cast<int32_t>(descriptorSetLayouts.size());

		pipelines.standard.build(&device,
		                         swapchain.getSwapchainExtent(),
		                         renderPass.getRenderPass(),
		                         materials
		                         .standard.getPipelineShaderStageCreateInfo(),
		                         descriptorSetLayouts.data(),
		                         descriptorCount);
	}

	~ObjViewer()
	{
		textures.albedo.release();
		models.ganesh.release();
		materials.standard.release();
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

		for (const auto& node : models.ganesh.nodes)
		{
			std::vector<vk::DescriptorSet> descriptorSets = {
				scene.getDescriptorSet(0),
				node->getDescriptorSet(),
				materials.standard.getDescriptorSet()
			};

			commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
			                           pipeline);

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                 pipelineLayout, 0,
			                                 static_cast<uint32_t>(
				                                 descriptorSets.size()),
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

			const auto vertexBuffer = models.ganesh.vertexBuffer;
			const auto indexBuffer = models.ganesh.indexBuffer;

			vk::DeviceSize offsets[] = {0};

			commandBuffer.
				bindVertexBuffers(0, 1, &vertexBuffer.buffer, offsets);

			commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
			                              vk::IndexType::eUint32);


			const auto size = node->indexCount;
			const auto indexStart = node->startIndex;

			commandBuffer.drawIndexed(size, 1, indexStart, 0, 0);
		}

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}
};

ObjViewer* objViewer;

int main()
{
	objViewer = new ObjViewer();
	objViewer->run();
	delete objViewer;
	return EXIT_SUCCESS;
}
