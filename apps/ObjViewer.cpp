#include "AppBase.h"
#include "Texture2D.h"
#include "BaseMaterial.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../3rdParty/tiny_obj_loader.h"
#include "../3rdParty/stb_image.h"

class ObjViewer : public mvk::AppBase
{
	void loadObjFile(std::vector<mvk::Vertex>& vertices,
	                 std::vector<uint16_t>& indices) const
	{
		const auto path = "assets/models/ganesha.obj";
		tinyobj::attrib_t attribute;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!LoadObj(&attribute, &shapes, &materials, &err, path,
		             "", true))
		{
			throw std::runtime_error("Failed to load obj file" + err + warn);
		}

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				mvk::Vertex vertex{};
				vertex.position = {
					attribute.vertices[3 * index.vertex_index + 0],
					attribute.vertices[3 * index.vertex_index + 1],
					attribute.vertices[3 * index.vertex_index + 2],
				};
				vertex.texCoord = {
					attribute.texcoords[2 * index.texcoord_index + 0],
					1.0f - attribute.texcoords[2 * index.texcoord_index + 1]
				};
				vertex.color = {1.0f, 1.0f, 1.0f};
				vertices.push_back(vertex);
				indices.push_back(static_cast<uint16_t>(indices.size()));
			}
		}
	}

public:
	ObjViewer()
	{
		std::vector<mvk::Vertex> vertices;
		std::vector<uint16_t> indices;

		loadObjFile(vertices, indices);

		const auto vertexBuffer = createVertexBufferObject(vertices);
		const auto indexBuffer = createIndexBufferObject(indices);

		const auto albedoPath = "assets/models/textures/Ganesha_BaseColor.jpg";

		auto albedo = mvk::Texture2D(albedoPath, vk::Format::eR8G8B8A8Srgb);
		const auto image = createTextureBufferObject(albedo.getPixels(),
		                                             albedo.getWidth(),
		                                             albedo.getHeight(),
		                                             albedo.getFormat());
		albedo.init(device, image);

		mvk::BaseMaterialDescription description;
		description.albedo = &albedo;

		auto material = mvk::BaseMaterial(device, description);

		std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts = {
			scene.getDescriptorSetLayout(),
			material.getDescriptorSetLayout()
		};

		mvk::GraphicPipeline graphicPipeline(device,
		                                     swapchain.getSwapchainExtent(),
		                                     renderPass.getRenderPass(),
		                                     material.
		                                     getPipelineShaderStageCreateInfo(),
		                                     descriptorSetLayouts.data(),
		                                     static_cast<int32_t>(
			                                     descriptorSetLayouts.
			                                     size()));

		const auto verticesCount = static_cast<uint32_t>(vertices.size());
		const auto indicesCount = static_cast<uint32_t>(indices.size());
		auto geometry = mvk::Geometry(vertexBuffer, verticesCount,
		                              indexBuffer, indicesCount);

		auto mesh = mvk::Mesh(&geometry, &material, &graphicPipeline);

		scene.addObject(&mesh);

		setupCommandBuffers();
	}
};

int main()
{
	ObjViewer app;
	app.run();
	app.terminate();
	return EXIT_SUCCESS;
}
