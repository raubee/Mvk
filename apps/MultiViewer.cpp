#include "AppBase.h"
#include "BaseMaterial.h"
#include "NormalMaterial.h"

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

		models.ganesh.loadFromFile(device, transferQueue, modelPath);

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

		const auto descriptorCount =
			static_cast<int32_t>(descriptorSetLayouts.size());

		pipelines.standard.build(device,
		                         swapchain.getSwapchainExtent(),
		                         renderPass.getRenderPass(),
		                         materials
		                         .standard.getPipelineShaderStageCreateInfo(),
		                         descriptorSetLayouts.data(),
		                         descriptorCount);
	}

	void loadPlane()
	{
		auto vertices = std::vector<mvk::Vertex>({
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

		auto indices = std::vector<uint16_t>({
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		});

		models.plane.verticesCount = static_cast<uint32_t>(vertices.size());
		models.plane.indicesCount = static_cast<uint32_t>(indices.size());

		const auto vSize =
			static_cast<vk::DeviceSize>(sizeof vertices.at(0) *
				models.plane.verticesCount);

		models.plane.vertexBuffer =
			device.transferDataSetToGpuBuffer(transferQueue, vertices.data(),
			                                  vSize,
			                                  vk::BufferUsageFlagBits::
			                                  eVertexBuffer);

		const auto iSize =
			static_cast<vk::DeviceSize>(sizeof indices.at(0) *
				models.plane.indicesCount);

		models.plane.indexBuffer =
			device.transferDataSetToGpuBuffer(transferQueue, indices.data(),
			                                  iSize,
			                                  vk::BufferUsageFlagBits::
			                                  eIndexBuffer);

		materials.normal.load(device);

		std::array<vk::DescriptorSetLayout, 1> descriptorSetLayouts = {
			mvk::Scene::getDescriptorSetLayout(device)
		};

		const auto descriptorCount =
			static_cast<int32_t>(descriptorSetLayouts.size());

		pipelines.normal.build(device,
		                       swapchain.getSwapchainExtent(),
		                       renderPass.getRenderPass(),
		                       materials.normal.
		                                 getPipelineShaderStageCreateInfo(),
		                       descriptorSetLayouts.data(),
		                       descriptorCount,
		                       vk::FrontFace::eClockwise);
	}

public:
	MultiViewer() : AppBase(mvk::AppInfo{
		.appName = "MultiViewer"
	})
	{
		loadGanesh();
		loadPlane();
	}

	~MultiViewer()
	{
		textures.albedo.release(device);
		models.ganesh.release(device);
		models.plane.release(device);
		materials.standard.release(device);
		materials.normal.release(device);
		pipelines.standard.release(device);
		pipelines.normal.release(device);
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

		drawGanesh(commandBuffer);
		drawPlane(commandBuffer);

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}

	void drawGanesh(const vk::CommandBuffer commandBuffer)
	{
		const auto graphicPipeline = pipelines.standard;
		const auto pipelineLayout = graphicPipeline.getPipelineLayout();
		const auto pipeline = graphicPipeline.getPipeline();
		const auto extent = swapchain.getSwapchainExtent();

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

		const auto vertexBuffer = models.ganesh.vertexBuffer;
		const auto indexBuffer = models.ganesh.indexBuffer;

		vk::DeviceSize offsets[] = {0};

		commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer.buffer, offsets);

		const auto size = models.ganesh.indicesCount;

		commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
		                              vk::IndexType::eUint16);

		commandBuffer.drawIndexed(size, 1, 0, 0, 0);
	}

	void drawPlane(const vk::CommandBuffer commandBuffer)
	{
		const auto graphicPipeline = pipelines.normal;
		const auto pipelineLayout = graphicPipeline.getPipelineLayout();
		const auto pipeline = graphicPipeline.getPipeline();
		const auto extent = swapchain.getSwapchainExtent();

		std::vector<vk::DescriptorSet> descriptorSets = {
			scene.getDescriptorSet(0)
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

		const auto vertexBuffer = models.plane.vertexBuffer;
		const auto indexBuffer = models.plane.indexBuffer;

		vk::DeviceSize offsets[] = {0};

		commandBuffer.bindVertexBuffers(0, 1, &vertexBuffer.buffer, offsets);

		const auto size = models.plane.indicesCount;

		commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
		                              vk::IndexType::eUint16);

		commandBuffer.drawIndexed(size, 1, 0, 0, 0);
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
