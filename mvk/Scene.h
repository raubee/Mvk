#pragma once

#include "Device.hpp"
#include "Camera.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

namespace mvk
{
	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class Scene
	{
		Device* ptrDevice;

		alloc::Buffer uniformBuffer;
		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;
		inline static vk::DescriptorSetLayout descriptorSetLayout;

		void createDescriptorPool(uint32_t size);
		void createDescriptorSets(uint32_t size);
		void createUniformBufferObject();
		void updateDescriptorSets();
		void updateUniformBufferObject(float time,
			vk::Extent2D extent) const;

	public:
		
		Camera camera;
		
		glm::mat4 modelMatrix;

		void setup(Device* device, uint32_t size, vk::Extent2D extent);
		void update(float time, vk::Extent2D extent) const;
		void release() const;

		static void createDescriptorSetLayout(Device* device)
		{
			const vk::DescriptorSetLayoutBinding uniformBufferLayoutBinding = {
				.binding = 0,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eVertex
			};

			std::array<vk::DescriptorSetLayoutBinding, 1> layoutBindings
				= { uniformBufferLayoutBinding };

			const vk::DescriptorSetLayoutCreateInfo
				descriptorSetLayoutCreateInfo = {
					.bindingCount = static_cast<uint32_t>(layoutBindings.size()
					),
					.pBindings = layoutBindings.data()
			};

			descriptorSetLayout = device->logicalDevice
			.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
		}

		static vk::DescriptorSetLayout getDescriptorSetLayout(Device* device)
		{
			if (!descriptorSetLayout)
			{
				createDescriptorSetLayout(device);
			}

			return descriptorSetLayout;
		}

		vk::DescriptorSet getDescriptorSet(const int i)
		{
			return descriptorSets[i];
		}
	};
}
