#define TINYOBJLOADER_IMPLEMENTATION

#define STBI_MSC_SECURE_CRT
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "Vertex.h"
#include "Model.h"
#include <filesystem>
#include <iostream>


namespace fs = std::filesystem;

using namespace mvk;

void Model::setup(const Device device)
{
	createModelMatrix(device);
	createDescriptorSetLayout(device);
	createDescriptorPool(device);
	createDescriptorSets(device);
}

void Model::createModelMatrix(Device device)
{
}

void Model::createDescriptorPool(const Device device)
{
	vk::DescriptorPoolSize descriptorPoolSize = {
		.descriptorCount = 1
	};

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		.maxSets = 1,
		.poolSizeCount =1,
		.pPoolSizes = &descriptorPoolSize
	};

	vk::Device(device).createDescriptorPool(descriptorPoolCreateInfo);
}

void Model::createDescriptorSets(Device device)
{
}

void Model::release(const Device device) const
{
	device.destroyBuffer(vertexBuffer);
	device.destroyBuffer(indexBuffer);

	vk::Device(device).destroyDescriptorSetLayout(descriptorSetLayout);
	vk::Device(device).destroyDescriptorPool(descriptorPool);
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

	if (indicesCount > 0)
	{
		const auto iSize =
			static_cast<vk::DeviceSize>(sizeof indices.at(0) * indicesCount);

		indexBuffer = device.transferDataSetToGpuBuffer(transferQueue,
		                                                indices.data(), iSize,
		                                                vk::BufferUsageFlagBits
		                                                ::
		                                                eIndexBuffer);
	}

	setup(device);
}

void Model::loadFromGltfFile(const char* filePath,
                             std::vector<Vertex>& vertices,
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
		throw std::runtime_error("Failed to parse glTF");
	}

	const auto iScene = model.defaultScene > -1 ? model.defaultScene : 0;
	const auto scene = model.scenes[iScene];

	for (auto iNode : scene.nodes)
	{
		const auto node = model.nodes[iNode];
		loadNode(nullptr, node, iNode, model, vertices, indices);
	}
}

void Model::loadNode(Node* parent, const tinygltf::Node node, uint32_t nodeId,
                     const tinygltf::Model model,
                     std::vector<Vertex>& vertices,
                     std::vector<uint16_t>& indices)
{
	Node* pNode = new Node
	{
		.id = nodeId,
		.parent = parent,
		.name = node.name.c_str()
	};

	for (auto child : node.children)
	{
		loadNode(pNode, model.nodes[child], child, model, vertices, indices);
	}

	// Node data
	if (node.mesh > -1)
	{
		const tinygltf::Mesh mesh = model.meshes[node.mesh];
		pNode->mesh = new Mesh();

		for (auto primitive : mesh.primitives)
		{
			const uint32_t vertexStart = static_cast<uint32_t>(vertices.size());
			uint32_t verticesCount = 0;
			uint32_t indicesCount = 0;

			const auto positionAccessor =
				model.accessors[primitive.attributes.find("POSITION")->second];
			const auto positionBufferView =
				model.bufferViews[positionAccessor.bufferView];
			const auto buffer = model.buffers[positionBufferView.buffer];
			const auto byteStride =
				positionAccessor.ByteStride(positionBufferView)
					? positionAccessor.ByteStride(positionBufferView) / sizeof(
						float)
					: tinygltf::GetComponentSizeInBytes(TINYGLTF_TYPE_VEC3);
			const auto posData =
				reinterpret_cast<const float*>(&buffer.data[
					positionAccessor.byteOffset +
					positionBufferView.byteOffset]);

			verticesCount = positionAccessor.count;
			for (auto v = 0; v < verticesCount; v++)
			{
				Vertex vertex{
					.position = glm::vec4(
						glm::make_vec3(&posData[v * byteStride]), 1.0f)
				};

				vertices.push_back(vertex);
			}

			const auto hasIndices = primitive.indices > -1;

			if (hasIndices)
			{
				const auto indicesAccessor = model.accessors[primitive.indices];
				const auto indicesBufferView =
					model.bufferViews[indicesAccessor.bufferView];

				indicesCount = indicesAccessor.count;

				const void* dataPtr = &(buffer.data[indicesAccessor.byteOffset
					+ indicesBufferView.byteOffset]);

				switch (indicesAccessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					{
						const uint32_t* buf = static_cast<const uint32_t*>(
							dataPtr);
						for (size_t index = 0; index < indicesAccessor.count;
						     index++)
						{
							indices.push_back(buf[index]);
						}
						break;
					}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					{
						const uint16_t* buf = static_cast<const uint16_t*>(
							dataPtr);
						for (size_t index = 0; index < indicesAccessor.count;
						     index++)
						{
							indices.push_back(buf[index] + vertexStart);
						}
						break;
					}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					{
						const uint8_t* buf = static_cast<const uint8_t*>(dataPtr
						);
						for (size_t index = 0; index < indicesAccessor.count;
						     index++)
						{
							indices.push_back(buf[index] + vertexStart);
						}
						break;
					}
				}
			}
		}
	}

	this->nodes.push_back(pNode);
}

void Model::loadFromObjFile(const char* filePath, std::vector<Vertex>& vertices,
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
