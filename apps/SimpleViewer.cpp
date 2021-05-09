#include "AppBase.h"
#include "Model.h"
#include "GraphicPipeline.h"
#include "NormalMaterial.h"

class SimpleViewer : public mvk::AppBase
{
public:
	struct Materials
	{
		mvk::NormalMaterial standard;
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
		scene.camera.setPerspective(45.0f, float(width) / float(height),
		                            0.1f, 100.0f);

		scene.camera.setType(Camera::CameraType::ORBIT);
		scene.camera.setLookAt(glm::vec3(0.0f, 0.5f, 0.0f));
		scene.camera.setDistance(1.5f);

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

		materials.standard.load(&device);

		const std::vector<vk::VertexInputBindingDescription> bindingDescription
			= {
				mvk::Vertex::getBindingDescription()
			};

		const auto& attributeDescriptions =
			mvk::Vertex::getAttributeDescriptions();

		const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			mvk::Scene::getDescriptorSetLayout(&device),
			mvk::Model::getDescriptorSetLayout(&device),
		};

		const auto shaderStageInfo =
			materials.standard.getPipelineShaderStageCreateInfo();

		const mvk::GraphicPipelineCreateInfo opaquePipelineCreateInfo =
		{
			.vertexInputBindingDescription = bindingDescription,
			.vertexInputAttributeDescription = attributeDescriptions,
			.renderPass = renderPass.getRenderPass(),
			.shaderStageCreateInfos = shaderStageInfo,
			.descriptorSetLayouts = descriptorSetLayouts,
			.frontFace = vk::FrontFace::eClockwise
		};

		pipelines.standard.build(&device, opaquePipelineCreateInfo);
	}

	~SimpleViewer()
	{
		models.plane.release();
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

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		                           pipeline);

		for (const auto& node : models.plane.nodes)
		{
			std::vector<vk::DescriptorSet> descriptorSets = {
				scene.getDescriptorSet(0),
				node->getDescriptorSet()
			};

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


			commandBuffer.bindIndexBuffer(indexBuffer.buffer, 0,
			                              vk::IndexType::eUint32);


			const auto size = node->indexCount;
			commandBuffer.drawIndexed(size, 1, 0, 0, 0);
		}

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
