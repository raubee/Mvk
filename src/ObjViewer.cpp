#include "ObjViewer.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "3rdParty/tiny_obj_loader.h"
#include "3rdParty/stb_image.h"

using namespace mvk;

ObjViewer::ObjViewer(
	const Context context,
	const vk::SurfaceKHR surface) : AppBase(context, surface)
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	loadObjFile(vertices, indices);

	const auto vertexBuffer = createVertexBufferObject(vertices);
	const auto indexBuffer = createIndexBufferObject(indices);

	const auto albedoPath = "assets/models/textures/Ganesha_BaseColor.jpg";

	auto albedo = Texture2D(albedoPath, vk::Format::eR8G8B8A8Srgb);
	loadTexture(albedo);

	BaseMaterialDescription description;
	description.albedo = &albedo;

	const auto material = new BaseMaterial(device, allocator, description);
	material->init(device, allocator, swapchain.getSwapchainSwainSize());

	GraphicPipeline opaquePipeline(device,
	                               swapchain.getSwapchainExtent(),
	                               renderPass.getRenderPass(),
	                               material->getPipelineShaderStageCreateInfo(),
	                               material->getDescriptorSetLayout(device));

	material->setGraphicPipeline(&opaquePipeline);
	material->createDescriptorPool(device, swapchain.getSwapchainSwainSize());

	graphicPipelines.push_back(&opaquePipeline);

	const auto geometry = new Geometry(vertexBuffer, vertices.size(),
	                                   indexBuffer, indices.size());

	auto mesh = Mesh(geometry, material);

	scene.addObject(&mesh);

	scene.init(device, allocator, swapchain.getSwapchainSwainSize());

	setupCommandBuffers();
}

void ObjViewer::loadObjFile(std::vector<mvk::Vertex>& vertices,
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
