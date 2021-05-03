#pragma once

#include "Model.h"
#include "./VulkanVma.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mvk
{
	class Scene
	{
		alloc::Buffer uniformBuffer;
		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;
		inline static vk::DescriptorSetLayout descriptorSetLayout;

	public:

		void setup(Device device, uint32_t size, vk::Extent2D extent);
		void update(Device device, float time, vk::Extent2D extent) const;
		void release(Device device) const;

		void createDescriptorPool(Device device, uint32_t size);
		void createDescriptorSets(Device device, uint32_t size);
		void createUniformBufferObject(Device device);
		void updateDescriptorSets(Device device);
		void updateUniformBufferObject(Device device, float time,
		                               vk::Extent2D extent) const;

		static void createDescriptorSetLayout(const Device device)
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

			descriptorSetLayout = vk::Device(device).createDescriptorSetLayout(
				descriptorSetLayoutCreateInfo);
		}

		static vk::DescriptorSetLayout getDescriptorSetLayout(
			const Device device)
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
