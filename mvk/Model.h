#pragma once

#include "Vertex.h"
#include "BaseMaterial.h"
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
	struct NodeUBO
	{
		glm::mat4 matrix;
	};

	struct Node
	{
		const char* name;

		int id;
		int matId;
		bool hasIndices;
		bool hasMesh;

		Node* parent = nullptr;
		std::vector<Node*> childNodes;

		uint32_t startVertex;
		uint32_t startIndex;

		uint32_t indexCount;
		uint32_t vertexCount;

		glm::mat4 matrix = glm::mat4(1);
		glm::vec3 translation = glm::vec3(0);
		glm::mat4 rotation = glm::mat4(1);
		glm::vec3 scale = glm::vec3(1);

		alloc::Buffer matrixBuffer;

		std::vector<vk::DescriptorSet> descriptorSets;

		void release(Device* device) const;

		void createLocalMatrixBuffer(Device* device);
		void writeDescriptorSets(Device* device);
		void updateLocalMatrixObject(Device* device) const;

		vk::DescriptorSet getDescriptorSet()
		{
			return descriptorSets[0];
		}

		glm::mat4 getLocalMatrix() const;
		glm::mat4 getMatrix() const;
	};

	class Model
	{
		Device* ptrDevice;

		alloc::Buffer modelMatrixBuffer;
		vk::DescriptorPool descriptorPool;

		inline static vk::DescriptorSetLayout descriptorSetLayout;

		void setupDescriptors();
		void createDescriptorPool();
		void createDescriptorSets();

		void loadTextures(vk::Queue transferQueue, tinygltf::Model model);

		void loadMaterials(tinygltf::Model model);

		void loadGltfNode(Node* parent, tinygltf::Node node, int nodeId,
		                  tinygltf::Model model,
		                  std::vector<Vertex>& vertices,
		                  std::vector<uint32_t>& indices);

		void loadFromGltfFile(vk::Queue transferQueue,
		                      const char* filePath,
		                      std::vector<Vertex>& vertices,
		                      std::vector<uint32_t>& indices);

		void loadFromObjFile(const char* filePath,
		                     std::vector<Vertex>& vertices,
		                     std::vector<uint32_t>& indices);

	public:

		std::string folder;

		std::vector<Texture2D*> textures;
		std::vector<Material*> materials;
		std::vector<Node*> nodes;

		alloc::Buffer vertexBuffer;
		alloc::Buffer indexBuffer;

		void loadRaw(Device* device, vk::Queue transferQueue,
		             std::vector<Vertex> vertices,
		             std::vector<uint32_t> indices);

		void loadFromFile(Device* device, vk::Queue transferQueue,
		                  const char* filePath);

		void release() const;

		static vk::DescriptorSetLayout getDescriptorSetLayout(Device* device)
		{
			if (!descriptorSetLayout)
			{
				createDescriptorSetLayout(device);
			}

			return descriptorSetLayout;
		}

		static void createDescriptorSetLayout(Device* device)
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

			descriptorSetLayout = device->logicalDevice
			                            .createDescriptorSetLayout(
				                            descriptorSetLayoutCreateInfo);
		}
	};
}
