#pragma once

#include "Vertex.h"
#include "Material.h"
#include "GraphicPipeline.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// OBJ
#include "../3rdParty/tiny_obj_loader.h"

//GLTF
#define STBI_MSC_SECURE_CRT
#include "../3rdParty/tiny_gltf.h"

namespace mvk
{
	struct Node;

	class Mesh
	{
		struct UniformBuffer
		{
			glm::mat4 modelMatrix;
		}
		uniformBuffer;

		std::vector<Material> materials;
	};

	struct Node
	{
		const uint32_t id;
		Node* parent = nullptr;
		const char* name;
		std::vector<Node*> childNodes;

		Mesh* mesh;
	};

	class Model
	{
		std::vector<Node*> nodes;

		std::vector<Material> materials;
		std::vector<Mesh> meshes;

		alloc::Buffer modelMatrixBuffer;
		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;
		inline static vk::DescriptorSetLayout descriptorSetLayout;

		void setup(Device device);
		void createModelMatrix(Device device);
		void createDescriptorPool(Device device);
		void createDescriptorSets(Device device);

		void loadFromGltfFile(const char* filePath,
		                      std::vector<Vertex>& vertices,
		                      std::vector<uint16_t>& indices);
		void loadNode(Node* parent, tinygltf::Node node, uint32_t nodeId,
		              tinygltf::Model model,
		              std::vector<Vertex>& vertices,
		              std::vector<uint16_t>& indices);

		void loadFromObjFile(const char* filePath,
		                     std::vector<Vertex>& vertices,
		                     std::vector<uint16_t>& indices);

	public:

		alloc::Buffer vertexBuffer;
		alloc::Buffer indexBuffer;

		uint32_t verticesCount;
		uint32_t indicesCount;


		void loadFromFile(Device device, vk::Queue transferQueue,
		                  const char* filePath);

		void release(Device device) const;

		static vk::DescriptorSetLayout getDescriptorSetLayout(
			const Device device)
		{
			if (!descriptorSetLayout)
			{
				createDescriptorSetLayout(device);
			}

			return descriptorSetLayout;
		}

		static void createDescriptorSetLayout(const Device device)
		{
			const vk::DescriptorSetLayoutBinding uniformBufferLayoutBinding = {
				.binding = 0,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eVertex
			};

			std::array<vk::DescriptorSetLayoutBinding, 1> layoutBindings
				= {uniformBufferLayoutBinding};

			const vk::DescriptorSetLayoutCreateInfo
				descriptorSetLayoutCreateInfo = {
					.bindingCount = static_cast<uint32_t>(layoutBindings.size()
					),
					.pBindings = layoutBindings.data()
				};

			descriptorSetLayout = vk::Device(device).createDescriptorSetLayout(
				descriptorSetLayoutCreateInfo);
		}
	};
}
