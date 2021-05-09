#pragma once

#include "Device.hpp"
#include "Shader.h"
#include "CubemapTexture.h"
#include "GraphicPipeline.h"

#include <glm/glm.hpp>

namespace mvk
{
	class Skybox
	{

		struct SkyboxVertex
		{
			glm::vec3 position;
			glm::vec2 texCoord;

			static vk::VertexInputBindingDescription
			getVertexInputBindingDescription()
			{
				vk::VertexInputBindingDescription vertexInputBindingDescription{
					.binding = 0,
					.stride = sizeof(SkyboxVertex),
					.inputRate = vk::VertexInputRate::eVertex
				};

				return vertexInputBindingDescription;
			}

			static std::vector<vk::VertexInputAttributeDescription>
			getVertexInputAttributeDescriptions()
			{
				std::vector<vk::VertexInputAttributeDescription>
					vertexInputAttributeDescriptions = {
						// Position
						{
							.location = 0,
							.binding = 0,
							.format = vk::Format::eR32G32B32Sfloat,
							.offset = offsetof(SkyboxVertex, position)
						},
						// UV0
						{
							.location = 1,
							.binding = 0,
							.format = vk::Format::eR32G32Sfloat,
							.offset = offsetof(SkyboxVertex, texCoord)
						}
					};

				return vertexInputAttributeDescriptions;
			}
		};

		Device* ptrDevice;

		inline static vk::DescriptorSetLayout descriptorSetLayout;

		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;

		Shader* vertexShader;
		Shader* fragmentShader;

		void loadShaders();
		void loadCubemap(vk::Queue transferQueue,
		                 std::array<std::string, 6> texturePaths);

		void createUniformBufferObject(vk::Queue transferQueue);
		void createSkyboxVertexBuffer(vk::Queue transferQueue);
		void createDescriptorPool();
		void createDescriptorSets();
		void updateDescriptorSets();

		void buildPipeline(vk::RenderPass renderPass);

		static void createDescriptorLayout(Device* device);

	public:
		
		struct UniformBufferObject
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		CubemapTexture cubemap;

		GraphicPipeline graphicPipeline;

		alloc::Buffer uniformBuffer;
		alloc::Buffer indexBuffer;
		alloc::Buffer vertexBuffer;

		uint32_t vertexCount;
		uint32_t indexCount;

		void create(Device* device, vk::Queue transferQueue,
		            vk::RenderPass renderPass,
		            std::array<std::string, 6> texturePaths);

		void release() const;

		vk::DescriptorSet getDescriptorSet(const int index)
		{
			return descriptorSets[index];
		}

		static vk::DescriptorSetLayout getDescriptorSetLayout(Device* device)
		{
			if (!descriptorSetLayout)
			{
				createDescriptorLayout(device);
			}

			return descriptorSetLayout;
		}
	};
}
