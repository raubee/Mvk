#include "AppBase.h"
#include "Texture2D.h"
#include "BaseMaterial.h"

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
		const auto modelPath = "assets/models/ganesha/ganesha.obj";
		//const auto modelPath = "assets/models/adamHead/adamHead.gltf";

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

	~ObjViewer()
	{
		textures.albedo.release(device);
		models.ganesh.release(device);
		materials.standard.release(device);
		pipelines.standard.release(device);
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
