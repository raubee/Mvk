#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../3rdParty/tiny_obj_loader.h"
#include "../3rdParty/stb_image.h"

using namespace mvk;

static void loadFromObjFile(const char* path,
                            std::vector<mvk::Vertex>& vertices,
                            std::vector<uint16_t>& indices);

void Mesh::loadFromFile(const vma::Allocator allocator,
                        const vk::Device device,
                        const vk::CommandPool commandPool,
                        const vk::Queue transferQueue,
                        const char* filePath)
{
	std::vector<mvk::Vertex> vertices;
	std::vector<uint16_t> indices;

	loadFromObjFile(filePath, vertices, indices);

	verticesCount = static_cast<uint32_t>(vertices.size());
	indicesCount = static_cast<uint32_t>(indices.size());

	const auto vSize =
		static_cast<vk::DeviceSize>(sizeof vertices.at(0) * verticesCount);

	vertexBuffer = alloc::transferDataSetToGpuBuffer(
		allocator, device, commandPool, transferQueue, vertices.data(), vSize,
		vk::BufferUsageFlagBits::eVertexBuffer);

	const auto iSize =
		static_cast<vk::DeviceSize>(sizeof indices.at(0) * indicesCount);

	indexBuffer = alloc::transferDataSetToGpuBuffer(
		allocator, device, commandPool, transferQueue, indices.data(), iSize,
		vk::BufferUsageFlagBits::eIndexBuffer);
}


void Mesh::release(const vma::Allocator allocator) const
{
	alloc::deallocateBuffer(allocator, vertexBuffer);
	alloc::deallocateBuffer(allocator, indexBuffer);
}

static void loadFromObjFile(const char* path,
                            std::vector<mvk::Vertex>& vertices,
                            std::vector<uint16_t>& indices)
{
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
