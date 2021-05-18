#pragma once

#include "Device.hpp"
#include "Skybox.h"
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
		glm::vec3 camPos;
	};

	class Scene
	{
		Device* ptrDevice;

		alloc::Buffer uniformBuffer;
		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;

		void createDescriptorPool();
		void createDescriptorSets();
		void createDescriptorSetLayout();
		void createUniformBufferObject();
		void updateDescriptorSets();
		void updateUniformBufferObject(float time, float deltaTime);
		
	public:

		vk::DescriptorSetLayout descriptorSetLayout;

		Camera camera;
		Skybox* skybox;

		glm::mat4 modelMatrix;

		void setup(Device* device, Skybox* skybox = nullptr);

		void update(float time, float deltaTime);
		void release() const;

		void renderSkybox(vk::CommandBuffer commandBuffer);

		vk::DescriptorSet getDescriptorSet(const int i)
		{
			return descriptorSets[i];
		}
	};
}
