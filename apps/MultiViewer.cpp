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
			scene.getDescriptorSetLayout(),
			materials.standard.getDescriptorSetLayout()
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


		models.ganesh.setMaterial(&materials.standard);
		models.ganesh.setGraphicPipeline(&pipelines.standard);

		scene.addObject(&models.ganesh);
	}

	void loadPlane()
	{
		auto vertices = std::vector<mvk::Vertex>({
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
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
			scene.getDescriptorSetLayout()
		};

		pipelines.normal.build(device,
		                       swapchain.getSwapchainExtent(),
		                       renderPass.getRenderPass(),
		                       materials.normal.
		                                 getPipelineShaderStageCreateInfo(),
		                       descriptorSetLayouts.data(),
		                       static_cast<int32_t>(
			                       descriptorSetLayouts.
			                       size()));

		models.plane.setMaterial(&materials.normal);
		models.plane.setGraphicPipeline(&pipelines.normal);

		scene.addObject(&models.plane);
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
		materials.standard.release(device);
		pipelines.standard.release(device);
		models.plane.release(device);
		materials.normal.release(device);
		pipelines.normal.release(device);
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
