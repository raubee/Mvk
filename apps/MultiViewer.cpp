#include "AppBase.h"
#include "Model.h"
#include "BaseMaterial.h"
#include "NormalMaterial.h"
#include "GraphicPipeline.h"

class MultiViewer : public mvk::AppBase
{
	struct Textures
	{
		mvk::Texture2D albedo;
	}
	textures;

	struct Materials
	{
		mvk::NormalMaterial normal;
		mvk::BaseMaterial standard;
	}
	materials;

	struct Models
	{
		mvk::Model ganesh;
		mvk::Model plane;
	}
	models;

	struct GraphicPipelines
	{
		mvk::GraphicPipeline normal;
		mvk::GraphicPipeline standard;
	}
	pipelines;

	void loadGanesh()
	{
		const auto modelPath = "assets/models/ganesha/ganesha.obj";

		models.ganesh.loadFromFile(&device, transferQueue, modelPath);

		const auto albedoPath =
			"assets/models/ganesha/textures/Ganesha_BaseColor.jpg";

		textures.albedo.loadFromFile(&device, transferQueue, albedoPath,
		                             vk::Format::eR8G8B8A8Srgb);

		mvk::BaseMaterialDescription description;
		description.baseColor = &textures.albedo;

		materials.standard.load(&device, description);

		const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			mvk::Scene::getDescriptorSetLayout(&device),
			mvk::Model::getDescriptorSetLayout(&device),
			mvk::BaseMaterial::getDescriptorSetLayout(&device)
		};

		const auto shaderStageInfo =
			materials.standard.getPipelineShaderStageCreateInfo();

		const mvk::GraphicPipelineCreateInfo opaquePipelineCreateInfo =
		{
			.extent = swapchain.getSwapchainExtent(),
			.renderPass = renderPass.getRenderPass(),
			.shaderStageCreateInfos = shaderStageInfo,
			.descriptorSetLayouts = descriptorSetLayouts,
			.frontFace = vk::FrontFace::eCounterClockwise
		};

		pipelines.standard.build(&device, opaquePipelineCreateInfo);
	}

	void loadPlane()
	{
		const auto vertices = std::vector<mvk::Vertex>({
			{
				{-0.5f, 0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
				{0.0f, 0.0f}
			},
			{
				{0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
				{1.0f, 0.0f}
			},
			{
				{0.5f, 0.0f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f},
				{1.0f, 1.0f}
			},
			{
				{-0.5f, 0.0f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f},
				{0.0f, 1.0f}
			},

			{
				{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
				{0.0f, 0.0f}
			},
			{
				{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f},
				{1.0f, 0.0f}
			},
			{
				{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f},
				{1.0f, 1.0f}
			},
			{
				{-0.5f, -0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f},
				{0.0f, 1.0f}
			}
		});

		const auto indices = std::vector<uint32_t>({
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		});

		models.plane.loadRaw(&device, transferQueue, vertices, indices);

		materials.normal.load(&device);

		const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			mvk::Scene::getDescriptorSetLayout(&device),
			mvk::Model::getDescriptorSetLayout(&device)
		};

		const auto shaderStageInfo =
			materials.normal.getPipelineShaderStageCreateInfo();

		const mvk::GraphicPipelineCreateInfo opaquePipelineCreateInfo =
		{
			.extent = swapchain.getSwapchainExtent(),
			.renderPass = renderPass.getRenderPass(),
			.shaderStageCreateInfos = shaderStageInfo,
			.descriptorSetLayouts = descriptorSetLayouts,
			.frontFace = vk::FrontFace::eClockwise
		};

		pipelines.normal.build(&device, opaquePipelineCreateInfo);
	}

public:
	MultiViewer() : AppBase(mvk::AppInfo{
		.appName = "MultiViewer"
	})
	{
		scene.camera.setPerspective(45.0f, float(width) / float(height),
		                            0.1f, 100.0f);

		scene.camera.setType(Camera::CameraType::ORBIT);
		scene.camera.setLookAt(glm::vec3(0.0f, 0.5f, 0.0f));
		scene.camera.setDistance(2.0f);

		loadGanesh();
		loadPlane();
	}

	~MultiViewer()
	{
		textures.albedo.release();
		models.ganesh.release();
		models.plane.release();
		materials.standard.release();
		materials.normal.release();
		pipelines.standard.release();
		pipelines.normal.release();
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

		drawGanesh(commandBuffer);
		drawPlane(commandBuffer);

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	void drawGanesh(const vk::CommandBuffer commandBuffer)
	{
		const auto graphicPipeline = pipelines.standard;
		const auto pipeline = graphicPipeline.getPipeline();
		const auto pipelineLayout = graphicPipeline.getPipelineLayout();

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		                           pipeline);

		for (const auto& node : models.ganesh.nodes)
		{
			std::vector<vk::DescriptorSet> descriptorSets = {
				scene.getDescriptorSet(0),
				node->getDescriptorSet(),
				materials.standard.getDescriptorSet()
			};

			const auto descriptorCount =
				static_cast<uint32_t>(descriptorSets.size());

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                 pipelineLayout, 0, descriptorCount,
			                                 descriptorSets.data(), 0, nullptr);

			const auto vertexBuffer = models.ganesh.vertexBuffer;
			const auto indexBuffer = models.ganesh.indexBuffer;

			vk::DeviceSize offsets[] = {0};

			commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer.buffer,
			                                offsets);

			commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
			                              vk::IndexType::eUint32);

			commandBuffer.drawIndexed(node->indexCount, 1,
			                          node->startIndex, 0, 0);
		}
	}

	void drawPlane(const vk::CommandBuffer commandBuffer)
	{
		const auto graphicPipeline = pipelines.normal;
		const auto pipeline = graphicPipeline.getPipeline();
		const auto pipelineLayout = graphicPipeline.getPipelineLayout();

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		                           pipeline);

		for (const auto& node : models.plane.nodes)
		{
			std::vector<vk::DescriptorSet> descriptorSets = {
				scene.getDescriptorSet(0),
				node->getDescriptorSet()
			};

			const auto descriptorCount = static_cast<uint32_t>(
				descriptorSets.size());

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			                                 pipelineLayout, 0, descriptorCount,
			                                 descriptorSets.data(), 0, nullptr);

			const auto vertexBuffer = models.plane.vertexBuffer;
			const auto indexBuffer = models.plane.indexBuffer;

			vk::DeviceSize offsets[] = {0};

			commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer.buffer,
			                                offsets);

			commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
			                              vk::IndexType::eUint32);

			commandBuffer.drawIndexed(node->indexCount, 1,
			                          node->startIndex, 0, 0);
		}
	}
};

MultiViewer* multiViewer;

int main()
{
	multiViewer = new MultiViewer();
	multiViewer->run();
	delete multiViewer;
	return EXIT_SUCCESS;
}
