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

// Node

void Node::release(Device* device) const
{
	device->destroyBuffer(matrixBuffer);
}

glm::mat4 Node::getLocalMatrix() const
{
	return glm::scale(glm::translate(rotation, translation), scale) * matrix;
}

glm::mat4 Node::getMatrix() const
{
	auto matrix = getLocalMatrix();
	auto parent = this->parent;

	while (parent)
	{
		matrix = parent->getLocalMatrix() * matrix;
		parent = parent->parent;
	}

	return matrix;
}

void Node::createLocalMatrixBuffer(Device* device)
{
	const auto size = sizeof(glm::mat4);

	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eUniformBuffer,
	};

	matrixBuffer =
		alloc::allocateCpuToGpuBuffer(device->allocator, bufferCreateInfo);
}

void Node::writeDescriptorSets(Device* device)
{
	const auto descriptorBufferInfo = vk::DescriptorBufferInfo{
		.buffer = matrixBuffer.buffer,
		.range = sizeof(NodeUBO)
	};

	for (const auto& descriptorSet : descriptorSets)
	{
		const auto writeDescriptorSet = vk::WriteDescriptorSet{
			.dstSet = descriptorSet,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = vk::DescriptorType::eUniformBuffer,
			.pBufferInfo = &descriptorBufferInfo
		};

		device->logicalDevice
		      .updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	}
}

void Node::updateLocalMatrixObject(Device* device) const
{
	auto matrix = getMatrix();

	mapDataToBuffer(device->allocator, matrixBuffer, &matrix,
	                sizeof matrix);
}


// Model
void Model::setupDescriptors()
{
	if (!descriptorSetLayout)
		createDescriptorSetLayout(ptrDevice);

	createDescriptorPool();
	createDescriptorSets();
}

void Model::createDescriptorPool()
{
	vk::DescriptorPoolSize descriptorPoolSize = {
		.descriptorCount = static_cast<uint32_t>(nodes.size())
	};

	const vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		.maxSets = static_cast<uint32_t>(nodes.size()),
		.poolSizeCount =1,
		.pPoolSizes = &descriptorPoolSize
	};

	descriptorPool = ptrDevice->logicalDevice
	                          .createDescriptorPool(descriptorPoolCreateInfo);
}

void Model::createDescriptorSets()
{
	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		.descriptorPool = descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &descriptorSetLayout
	};

	for (const auto& node : nodes)
	{
		node->descriptorSets = ptrDevice->logicalDevice.
		                                  allocateDescriptorSets(
			                                  descriptorSetAllocateInfo);

		node->createLocalMatrixBuffer(ptrDevice);
		node->updateLocalMatrixObject(ptrDevice);
		node->writeDescriptorSets(ptrDevice);
	}
}

void Model::release() const
{
	for (const auto& node : nodes)
	{
		node->release(ptrDevice);
	}

	for (const auto& texture : textures)
	{
		texture->release();
	}

	for (const auto& material : materials)
	{
		material->release();
	}

	ptrDevice->destroyBuffer(vertexBuffer);
	ptrDevice->destroyBuffer(indexBuffer);

	ptrDevice->logicalDevice.destroyDescriptorPool(descriptorPool);

	// Fix: Static destroy
	if (descriptorSetLayout)
		ptrDevice->logicalDevice.
		           destroyDescriptorSetLayout(descriptorSetLayout);
	descriptorSetLayout = nullptr;
}

void Model::loadRaw(Device* device, const vk::Queue transferQueue,
                    std::vector<Vertex> vertices,
                    std::vector<uint32_t> indices)
{
	this->ptrDevice = device;

	const auto node = new Node;

	node->startVertex = 0;
	node->startIndex = 0;

	node->indexCount = static_cast<uint32_t>(indices.size());
	node->vertexCount = static_cast<uint32_t>(vertices.size());

	nodes.push_back(node);

	const auto vSize =
		static_cast<vk::DeviceSize>(sizeof vertices.at(0) * node->vertexCount);

	vertexBuffer = ptrDevice->transferDataSetToGpuBuffer(transferQueue,
	                                                     vertices.data(), vSize,
	                                                     vk::BufferUsageFlagBits
	                                                     ::
	                                                     eVertexBuffer);

	if (node->indexCount > 0)
	{
		const auto iSize =
			static_cast<vk::DeviceSize>(sizeof indices.at(0) * node->indexCount
			);

		indexBuffer = ptrDevice->transferDataSetToGpuBuffer(transferQueue,
		                                                    indices.data(),
		                                                    iSize,
		                                                    vk::
		                                                    BufferUsageFlagBits
		                                                    ::
		                                                    eIndexBuffer);
	}

	setupDescriptors();
}


void Model::loadFromFile(Device* device, const vk::Queue transferQueue,
                         const char* filePath)
{
	this->ptrDevice = device;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	const fs::path path = filePath;

	if (path.extension() == ".obj")
	{
		loadFromObjFile(filePath, vertices, indices);
	}
	else if (path.extension() == ".gltf" || path.extension() == ".glb")
	{
		loadFromGltfFile(transferQueue, filePath, vertices, indices);
	}
	else
	{
		std::cerr << "Error : Loading" << filePath
			<< ": extention not supported" << std::endl;
		return;
	}

	const auto verticesCount = static_cast<uint32_t>(vertices.size());
	const auto indicesCount = static_cast<uint32_t>(indices.size());

	const auto vSize =
		static_cast<vk::DeviceSize>(sizeof vertices.at(0) * verticesCount);

	vertexBuffer = device->transferDataSetToGpuBuffer(transferQueue,
	                                                  vertices.data(), vSize,
	                                                  vk::BufferUsageFlagBits::
	                                                  eVertexBuffer);

	if (indicesCount > 0)
	{
		const auto iSize =
			static_cast<vk::DeviceSize>(sizeof indices.at(0) * indicesCount);

		indexBuffer = device->transferDataSetToGpuBuffer(transferQueue,
		                                                 indices.data(), iSize,
		                                                 vk::BufferUsageFlagBits
		                                                 ::
		                                                 eIndexBuffer);
	}

	setupDescriptors();
}

void Model::loadFromGltfFile(const vk::Queue transferQueue,
                             const char* filePath,
                             std::vector<Vertex>& vertices,
                             std::vector<uint32_t>& indices)
{
	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	const fs::path path = filePath;
	const fs::path parent = path.parent_path();

	folder = parent.string();

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

	loadTextures(transferQueue, model);
	loadMaterials(model);

	for (const auto& iNode : scene.nodes)
	{
		const auto node = model.nodes[iNode];
		loadGltfNode(nullptr, node, iNode, model, vertices, indices);
	}
}

void Model::loadFromObjFile(const char* filePath, std::vector<Vertex>& vertices,
                            std::vector<uint32_t>& indices)
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
		auto node = new Node();

		node->indexCount = static_cast<uint32_t>(shape.mesh.indices.size());
		node->startVertex = static_cast<uint32_t>(vertices.size());

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

			vertex.normal = {
				attribute.normals[3 * index.normal_index + 0],
				attribute.normals[3 * index.normal_index + 1],
				attribute.normals[3 * index.normal_index + 2],
			};

			vertex.color = {1.0f, 1.0f, 1.0f};

			vertices.push_back(vertex);

			indices.push_back(static_cast<uint16_t>(indices.size()));
		}

		nodes.push_back(node);
	}
}

void Model::loadGltfNode(Node* parent,
                         const tinygltf::Node node,
                         int nodeId,
                         const tinygltf::Model model,
                         std::vector<Vertex>& vertices,
                         std::vector<uint32_t>& indices)
{
	Node* pNode = new Node
	{
		.name = node.name.c_str(),
		.id = nodeId,
		.parent = parent
	};

	if (parent != nullptr)
	{
		parent->childNodes.push_back(pNode);
	}

	// Matrix
	if (node.matrix.size() == 16)
	{
		pNode->matrix = glm::make_mat4(node.matrix.data());
	}

	// Translation
	if (node.translation.size() == 3)
	{
		pNode->translation = glm::make_vec3(node.translation.data());
	}

	// Rotation
	if (node.rotation.size() == 4)
	{
		const glm::quat q = glm::make_quat(node.rotation.data());
		pNode->rotation = glm::mat4(q);
	}

	// Scale
	if (node.scale.size() == 3)
	{
		pNode->scale = glm::make_vec3(node.scale.data());
	}

	for (const auto& child : node.children)
	{
		loadGltfNode(pNode, model.nodes[child], child, model, vertices,
		             indices);
	}

	pNode->hasMesh = node.mesh > -1;

	// Node data
	if (pNode->hasMesh)
	{
		const auto& mesh = model.meshes[node.mesh];

		for (const auto& primitive : mesh.primitives)
		{
			pNode->matId = primitive.material;
			pNode->startVertex = static_cast<uint32_t>(vertices.size());
			pNode->startIndex = static_cast<uint32_t>(indices.size());

			// Position
			const auto& posAccessor =
				model.accessors[primitive.attributes.find("POSITION")->second];

			const auto& posBufferView =
				model.bufferViews[posAccessor.bufferView];

			const auto& posBuffer = model.buffers[posBufferView.buffer];

			const auto& posStride =
				posAccessor.ByteStride(posBufferView)
					? posAccessor.ByteStride(posBufferView) / sizeof(
						float)
					: tinygltf::GetComponentSizeInBytes(TINYGLTF_TYPE_VEC3);

			const auto& posData =
				reinterpret_cast<const float*>(&posBuffer.data[
					posAccessor.byteOffset +
					posBufferView.byteOffset]);

			pNode->vertexCount = static_cast<uint32_t>(posAccessor.count);

			// UVs
			const float* uv1Data = nullptr;
			uint32_t uv1Stride = 0;

			if (primitive.attributes.find("TEXCOORD_0") != primitive
			                                               .attributes.end())
			{
				const auto& uv1Accessor =
					model.accessors[primitive
					                .attributes.find("TEXCOORD_0")->second];

				const auto& uv1BufferView =
					model.bufferViews[uv1Accessor.bufferView];

				uv1Stride =
					uv1Accessor.ByteStride(uv1BufferView)
						? uv1Accessor.ByteStride(uv1BufferView) / sizeof(float)
						: tinygltf::GetComponentSizeInBytes(TINYGLTF_TYPE_VEC2);

				uv1Data = reinterpret_cast<const float*>(&model.buffers[
					uv1BufferView.buffer].data[
					uv1Accessor.byteOffset + uv1BufferView.byteOffset]);
			}

			// Normal
			const float* normalData = nullptr;
			uint32_t normalStride = 0;

			if (primitive.attributes.find("NORMAL") != primitive
			                                           .attributes.end())
			{
				const auto& normalAccessor =
					model.accessors[primitive
					                .attributes.find("NORMAL")->second];

				const auto& normalBufferView =
					model.bufferViews[normalAccessor.bufferView];

				normalStride =
					normalAccessor.ByteStride(normalBufferView)
						? normalAccessor.ByteStride(normalBufferView) / sizeof(
							float)
						: tinygltf::GetComponentSizeInBytes(TINYGLTF_TYPE_VEC3);

				normalData = reinterpret_cast<const float*>(&model.buffers[
					normalBufferView.buffer].data[
					normalAccessor.byteOffset + normalBufferView.byteOffset]);
			}

			// Create vertices
			for (uint32_t v = 0; v < pNode->vertexCount; v++)
			{
				Vertex vertex{
					.position = glm::make_vec3(&posData[v * posStride]),
					.color = glm::vec3(0),
					.normal = normalData
						          ? glm::make_vec3(
							          &normalData[v * normalStride])
						          : glm::vec3(0),
					.texCoord = uv1Data
						            ? glm::make_vec2(&uv1Data[v * uv1Stride])
						            : glm::vec2(0)
				};

				vertices.push_back(vertex);
			}

			pNode->hasIndices = primitive.indices > -1;

			if (pNode->hasIndices)
			{
				const auto& accessor = model.accessors[primitive.indices];
				const auto& bufferView = model.bufferViews[accessor.bufferView];
				const auto& buffer = model.buffers[bufferView.buffer];

				pNode->indexCount = static_cast<uint32_t>(accessor.count);

				const void* dataPtr = &(buffer.data[accessor.byteOffset
					+ bufferView.byteOffset]);

				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					{
						const auto buf = static_cast<const uint32_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indices.push_back(buf[index] + pNode->startVertex);
						}
						break;
					}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					{
						const auto buf = static_cast<const uint16_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indices.push_back(buf[index] + pNode->startVertex);
						}
						break;
					}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					{
						const auto buf = static_cast<const uint8_t*>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							indices.push_back(buf[index] + pNode->startVertex);
						}
						break;
					}
				}
			}
		}
	}

	this->nodes.push_back(pNode);
}

void Model::loadTextures(const vk::Queue transferQueue,
                         const tinygltf::Model model)
{
	for (const auto& image : model.images)
	{
		auto path = folder + "/" + image.uri;

		auto texture = new Texture2D;

		try
		{
			texture->loadRaw(ptrDevice, transferQueue,
			                 image.image.data(), image.width, image.height);
		}
		catch (std::runtime_error e)
		{
			std::cerr << "Failed to load: " << path;
			continue;
		}

		textures.push_back(texture);
	}
}

void Model::loadMaterials(const tinygltf::Model model)
{
	for (const auto& mat : model.materials)
	{
		AlphaMode alphaMode;
		auto alpha = mat.alphaMode;
		
		if (alpha == "BLEND")
		{
			alphaMode = AlphaMode::ALPHA_BLEND;
		}
		else if (alpha == "MASK")
		{
			alphaMode = AlphaMode::ALPHA_CUTOFF;
		}
		else // Opaque or default
		{
			alphaMode = AlphaMode::NO_ALPHA;
		}

		BaseMaterialDescription materialDescription = {
			.alphaMode = alphaMode
		};

		// BaseColor
		if (mat.pbrMetallicRoughness.baseColorTexture.index > -1)
		{
			const auto& id =
				model.textures[mat.pbrMetallicRoughness.baseColorTexture.index];
			materialDescription.baseColor = textures[id.source];
		}

		// Normal
		if (mat.normalTexture.index > -1)
		{
			const auto& id = model.textures[mat.normalTexture.index];
			materialDescription.normal = textures[id.source];
		}

		// MetallicRoughness
		if (mat.pbrMetallicRoughness.metallicRoughnessTexture.index > -1)
		{
			const auto& id =
				model.textures[
					mat.pbrMetallicRoughness.metallicRoughnessTexture.index];
			materialDescription.metallicRoughness = textures[id.source];
		}

		auto material = new BaseMaterial;

		material->load(ptrDevice, materialDescription);

		materials.push_back(material);
	}
}
