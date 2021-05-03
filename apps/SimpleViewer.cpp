#include "AppBase.h"
#include "BaseMaterial.h"

class SimpleViewer : public mvk::AppBase
{
public:
	struct Textures
	{
		mvk::Texture2D lena;
	}
	textures;

	struct Materials
	{
		mvk::BaseMaterial standard;
	}
	materials;

	struct Models
	{
		mvk::Model plane;
	}
	models;

	struct Pipelines
	{
		mvk::GraphicPipeline standard;
	}
	pipelines;

	SimpleViewer() : AppBase(mvk::AppInfo
		{
			.appName = "SimpleViewer"
		})
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
				{0.5f, 0.0f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f},
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
			device.transferDataSetToGpuBuffer(transferQueue,
			                                  vertices.data(), vSize,
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

		const auto path = "assets/textures/lena.jpg";

		textures.lena.loadFromFile(device, transferQueue, path,
		                           vk::Format::eR8G8B8A8Srgb);

		mvk::BaseMaterialDescription description;
		description.albedo = &textures.lena;

		materials.standard.load(device, description);

		std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts = {
			mvk::Scene::getDescriptorSetLayout(device),
			mvk::BaseMaterial::getDescriptorSetLayout(device)
		};

		pipelines.standard.build(device,
		                         swapchain.getSwapchainExtent(),
		                         renderPass.getRenderPass(),
		                         materials.standard.
		                                   getPipelineShaderStageCreateInfo(),
		                         descriptorSetLayouts.data(),
		                         static_cast<uint32_t>(descriptorSetLayouts.
			                         size()));
	}

	~SimpleViewer()
	{
		models.plane.release(device);
		materials.standard.release(device);
		pipelines.standard.release(device);
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

		const auto vertexBuffer = models.plane.vertexBuffer;
		const auto indexBuffer = models.plane.indexBuffer;

		vk::DeviceSize offsets[] = {0};

		commandBuffer.bindVertexBuffers(0, 1,
		                                &vertexBuffer.buffer,
		                                offsets);


		const auto size = models.plane.indicesCount;

		commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
		                              vk::IndexType::eUint16);

		commandBuffer.drawIndexed(size, 1, 0, 0, 0);


		commandBuffer.endRenderPass();
		commandBuffer.end();
	}
};

SimpleViewer* simpleViewer;

int main()
{
	simpleViewer = new SimpleViewer();
	simpleViewer->run();
	delete simpleViewer;
	return EXIT_SUCCESS;
}
