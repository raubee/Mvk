#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../3rdParty/tiny_obj_loader.h"

#define STBI_MSC_SECURE_CRT
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../3rdParty/tiny_gltf.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

using namespace mvk;

static void loadFromObjFile(const char* filePath,
                            std::vector<mvk::Vertex>& vertices,
                            std::vector<uint16_t>& indices);

static int loadFromGltfFile(const char* filePath,
                            std::vector<mvk::Vertex>& vertices,
                            std::vector<uint16_t>& indices)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	const fs::path path = filePath;

	auto ret = false;

	if (path.extension() == ".gltf")
	{
		ret = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);
	}
	else if (path.extension() == ".glb")
	{
		ret = loader.LoadBinaryFromFile(&model, &err, &warn, filePath);
	}

	if (!warn.empty())
	{
		printf("Warn: %s\n", warn.c_str());
	}

	if (!err.empty())
	{
		printf("Err: %s\n", err.c_str());
	}

	if (!ret)
	{
		printf("Failed to parse glTF\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void Model::loadFromFile(const Device device, const vk::Queue transferQueue,
                         const char* filePath)
{
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;

	const fs::path path = filePath;

	if (path.extension() == ".obj")
	{
		loadFromObjFile(filePath, vertices, indices);
	}
	else if (path.extension() == ".gltf" || path.extension() == ".glb")
	{
		loadFromGltfFile(filePath, vertices, indices);
	}
	else
	{
		std::cerr << "Error : Loading" << filePath
			<< ": extention not supported" << std::endl;
		return;
	}

	verticesCount = static_cast<uint32_t>(vertices.size());
	indicesCount = static_cast<uint32_t>(indices.size());

	const auto vSize =
		static_cast<vk::DeviceSize>(sizeof vertices.at(0) * verticesCount);

	vertexBuffer = device.transferDataSetToGpuBuffer(transferQueue,
	                                                 vertices.data(), vSize,
	                                                 vk::BufferUsageFlagBits::
	                                                 eVertexBuffer);

	const auto iSize =
		static_cast<vk::DeviceSize>(sizeof indices.at(0) * indicesCount);

	indexBuffer = device.transferDataSetToGpuBuffer(transferQueue,
	                                                indices.data(), iSize,
	                                                vk::BufferUsageFlagBits::
	                                                eIndexBuffer);
}


void Model::release(const Device device) const
{
	device.destroyBuffer(vertexBuffer);
	device.destroyBuffer(indexBuffer);
}

static void loadFromObjFile(const char* filePath,
                            std::vector<mvk::Vertex>& vertices,
                            std::vector<uint16_t>& indices)
{
	tinyobj::attrib_t attribute;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!LoadObj(&attribute, &shapes, &materials, &err, filePath,
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
