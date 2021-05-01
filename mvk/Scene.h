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
		std::vector<Model*> models;

		alloc::Buffer uniformBuffer;
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;

	public:
		Scene() : models(0)
		{
		}

		void setup(Device device, uint32_t size, vk::Extent2D extent);
		void update(Device device, float time, vk::Extent2D extent) const;
		void release(Device device) const;

		void createDescriptorSetLayout(Device device);
		void createDescriptorPool(Device device, uint32_t size);
		void createDescriptorSets(Device device, uint32_t size);
		void createUniformBufferObject(Device device);
		void updateDescriptorSets(Device device);
		void updateUniformBufferObject(Device device, float time,
		                               vk::Extent2D extent) const;

		void addObject(Model* mesh) { models.push_back(mesh); }
		std::vector<Model*> getObjects() const { return models; }

		vk::DescriptorSetLayout getDescriptorSetLayout() const
		{
			return descriptorSetLayout;
		}

		vk::DescriptorSet getDescriptorSet(const int i)
		{
			return descriptorSets[i];
		}
	};
}
