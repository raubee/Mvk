#pragma once

#include "Mesh.h"
#include "VulkanVma.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mvk
{
	class Scene
	{
		std::vector<Mesh*> meshes;

		alloc::Buffer uniformBuffer;
		vk::DescriptorSetLayout descriptorSetLayout;
		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;

	public:
		Scene() : meshes(0)
		{
		}

		void init(vk::Device device,
		          vma::Allocator allocator,
		          uint32_t size,
		          vk::Extent2D extent);

		void update(vma::Allocator allocator, float time, vk::Extent2D extent);

		void release(vk::Device device, vma::Allocator allocator);

		void createDescriptorSetLayout(vk::Device device);
		void createDescriptorPool(vk::Device device, uint32_t size);
		void createDescriptorSets(vk::Device device, uint32_t size);
		void createUniformBufferObject(vma::Allocator allocator);
		void updateDescriptorSets(vk::Device device);
		void updateUniformBufferObject(vma::Allocator allocator, float time,
		                               vk::Extent2D extent) const;

		void addObject(Mesh* mesh) { meshes.push_back(mesh); }
		std::vector<Mesh*> getObjects() const { return meshes; }

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
