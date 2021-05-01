#include "AppBase.h"
#include "NormalMaterial.h"

class SimpleViewer : public mvk::AppBase
{
public:
	struct Materials
	{
		mvk::NormalMaterial normal;
	}
	materials;

	struct Models
	{
		mvk::Model plane;
	}
	models;

	struct Pipelines
	{
		mvk::GraphicPipeline normal;
	}
	pipelines;

	SimpleViewer() : AppBase(mvk::AppInfo
		{
			.appName = "SimpleViewer"
		})
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

	~SimpleViewer()
	{
		models.plane.release(device);
		materials.normal.release(device);
		pipelines.normal.release(device);
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
