#include "AppBase.h"
#include "Model.h"
#include "BaseMaterial.h"
#include "NormalMaterial.h"
#include "GraphicPipeline.h"

class MultiViewer : public mvk::AppBase
{
	mvk::Skybox skybox;

	struct Textures
	{
		mvk::Texture2D albedo;
		mvk::Texture2D normal;
		mvk::Texture2D roughness;
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
		const auto albedoPath =
			"assets/models/ganesha/textures/Ganesha_BaseColor.jpg";
		const auto normalPath =
			"assets/models/ganesha/textures/Ganesha_Normal.jpg";
		const auto roughnessPath =
			"assets/models/ganesha/textures/Ganesha_Roughness.jpg";

		models.ganesh.loadFromFile(&device, transferQueue, modelPath);

		textures.albedo.loadFromFile(&device, transferQueue, albedoPath,
		                             vk::Format::eR8G8B8A8Unorm);

		textures.normal.loadFromFile(&device, transferQueue, normalPath,
		                             vk::Format::eR8G8B8A8Unorm);

		textures.roughness.loadFromFile(&device, transferQueue, roughnessPath,
		                                vk::Format::eR8G8B8A8Unorm);

		mvk::BaseMaterial::BaseMaterialDescription description{
			.constants{
				.baseColorTextureSet = 0,
				.normalTextureSet = 0,
			},
			.baseColor = &textures.albedo,
			.normal = &textures.normal,
			.metallicRoughness = &textures.roughness,
		};

		materials.standard.load(&device, description);

		const std::vector<vk::VertexInputBindingDescription> bindingDescription
			= {
				mvk::Vertex::getBindingDescription()
			};

		const auto& attributeDescriptions =
			mvk::Vertex::getAttributeDescriptions();

		const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			scene.descriptorSetLayout,
			mvk::Model::getDescriptorSetLayout(&device),
			mvk::BaseMaterial::getDescriptorSetLayout(&device)
		};

		const auto shaderStageInfo =
			materials.standard.getPipelineShaderStageCreateInfo();

		const std::vector<vk::PushConstantRange> pushConstantRanges = {
			mvk::BaseMaterial::getPushConstantRange()
		};

		const mvk::GraphicPipelineCreateInfo opaquePipelineCreateInfo =
		{
			.vertexInputBindingDescription = bindingDescription,
			.vertexInputAttributeDescription = attributeDescriptions,
			.renderPass = renderPass.renderPass,
			.shaderStageCreateInfos = shaderStageInfo,
			.descriptorSetLayouts = descriptorSetLayouts,
			.pushConstantRanges = pushConstantRanges,
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

		const std::vector<vk::VertexInputBindingDescription> bindingDescription
			= {
				mvk::Vertex::getBindingDescription()
			};

		const auto& attributeDescriptions =
			mvk::Vertex::getAttributeDescriptions();

		const std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
			scene.descriptorSetLayout,
			mvk::Model::getDescriptorSetLayout(&device)
		};

		const auto shaderStageInfo =
			materials.normal.getPipelineShaderStageCreateInfo();

		const mvk::GraphicPipelineCreateInfo opaquePipelineCreateInfo =
		{
			.vertexInputBindingDescription = bindingDescription,
			.vertexInputAttributeDescription = attributeDescriptions,
			.renderPass = renderPass.renderPass,
			.shaderStageCreateInfos = shaderStageInfo,
			.descriptorSetLayouts = descriptorSetLayouts,	
			.frontFace = vk::FrontFace::eClockwise,
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

		const std::array<std::string, 6> skyboxTexturePaths = {
			"assets/textures/skybox/right.jpg",
			"assets/textures/skybox/left.jpg",
			"assets/textures/skybox/top.jpg",
			"assets/textures/skybox/bottom.jpg",
			"assets/textures/skybox/front.jpg",
			"assets/textures/skybox/back.jpg"
		};

		skybox.create(&device, transferQueue, renderPass.renderPass,
		              skyboxTexturePaths);

		scene.setup(&device, &skybox);

		loadGanesh();
		loadPlane();
	}

	~MultiViewer()
	{
		skybox.release();
		textures.albedo.release();
		textures.normal.release();
		textures.roughness.release();
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
			.renderPass = renderPass.renderPass,
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

		scene.renderSkybox(commandBuffer);

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

			commandBuffer.pushConstants(pipelineLayout,
			                            vk::ShaderStageFlagBits::eFragment, 0,
			                            sizeof(mvk::BaseMaterial::PushConstants
			                            ),
			                            &materials.standard.constants);

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
