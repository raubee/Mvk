#include "ObjViewer.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "3rdParty/tiny_obj_loader.h"
#include "3rdParty/stb_image.h"
#include "BaseMaterial.h"
#include "Geometry.h"

void ObjViewer::setup(
	const mvk::Context context,
	const vk::SurfaceKHR surface)
{
	loadObjFile();

	const auto albedoChalet = "assets/models/textures/Ganesha_BaseColor.jpg";

	const mvk::BaseMaterialDescription materialDescription = {
		.albedo = albedoChalet
	};

	const auto chaletMat = new mvk::BaseMaterial();
	chaletMat->setDescription(materialDescription);

	const auto chaletGeo = new mvk::Geometry(&vertices, &indices);
	const auto chalet = new mvk::Mesh(chaletGeo, chaletMat);

	scene.addObject(chalet);
	AppBase::setup(context, surface);
}

void ObjViewer::loadObjFile()
{
	const auto modelPath = "assets/models/ganesha.obj";
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelPath,
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
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2],
			};
			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
			vertex.color = {1.0f, 1.0f, 1.0f};
			vertices.push_back(vertex);
			indices.push_back(indices.size());
		}
	}
}
