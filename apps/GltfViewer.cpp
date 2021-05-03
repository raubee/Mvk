#include "AppBase.h"
#include "Texture2D.h"
#include "BaseMaterial.h"

class GltfViewer : public mvk::AppBase
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
		const auto modelPath = "assets/models/cube/cube1.gltf";
		//const auto modelPath = "assets/models/buster_drone/scene.gltf";

		models.scene.loadFromFile(device, transferQueue, modelPath);

		const auto albedoPath =
			"assets/models/ganesha/textures/Ganesha_BaseColor.jpg";

		textures.albedo.loadFromFile(device, transferQueue, albedoPath,
			vk::Format::eR8G8B8A8Srgb);

		mvk::BaseMaterialDescription description;
		description.albedo = &textures.albedo;

		materials.standard.load(device, description);

		std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts = {
			mvk::Scene::getDescriptorSetLayout(device),
			mvk::BaseMaterial::getDescriptorSetLayout(device)
		};

		pipelines.standard.build(device,
			swapchain.getSwapchainExtent(),
			renderPass.getRenderPass(),
			materials
			.standard.getPipelineShaderStageCreateInfo(),
			descriptorSetLayouts.data(),
			static_cast<int32_t>(
				descriptorSetLayouts.
				size()));
	}

	~GltfViewer()
	{
		textures.albedo.release(device);
		models.scene.release(device);
		materials.standard.release(device);
		pipelines.standard.release(device);
	}

	void buildCommandBuffer(const vk::CommandBuffer commandBuffer,
		const vk::Framebuffer framebuffer) override
	{
		const auto extent = swapchain.getSwapchainExtent();

		const vk::CommandBufferBeginInfo commandBufferBeginInfo{};
		commandBuffer.begin(commandBufferBeginInfo);

		const std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		std::array<vk::ClearValue, 2> clearValues{};
		clearValues[0].setColor(clearColor);
		clearValues[1].setDepthStencil({ 1.0f, 0 });

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

		std::vector<vk::DescriptorSet> descriptorSets = {
			scene.getDescriptorSet(0),
			materials.standard.getDescriptorSet(0)
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

		const auto vertexBuffer = models.scene.vertexBuffer;
		const auto indexBuffer = models.scene.indexBuffer;

		vk::DeviceSize offsets[] = { 0 };

		commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer.buffer, offsets);

		const auto indexCount = models.scene.indicesCount;

		// Has indices ?
		if(indexCount > 0)
		{
			commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
				vk::IndexType::eUint16);

			commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
		}
		else
		{
			const auto vertexCount = models.scene.verticesCount;
			commandBuffer.draw(vertexCount, 1, 0, 0);
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
