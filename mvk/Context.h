#pragma once

#include "ContextCreateInfo.h"

namespace mvk
{
	class Context
	{
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::PhysicalDeviceFeatures deviceFeatures;

		uint32_t graphicsQueueFamilyIndex;
		uint32_t transferQueueFamilyIndex;

		vk::Queue graphicsQueue;
		vk::Queue transferQueue;
		vk::Queue presentQueue;

	public:

		vk::Instance getInstance() const { return instance; }
		vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }
		vk::Device getDevice() const { return device; }

		uint32_t getGraphicsQueueFamilyIndex() const
		{
			return graphicsQueueFamilyIndex;
		}

		uint32_t getTransferQueueFamilyIndex() const
		{
			return transferQueueFamilyIndex;
		}

		vk::Queue getGraphicsQueue() const { return graphicsQueue; }
		vk::Queue getPresentQueue() const { return presentQueue; }
		vk::Queue getTransferQueue() const { return transferQueue; }

		void filterAvailableLayers(std::vector<const char*>& layers);
		void filterDeviceExtensions(std::vector<const char*>& extensions) const;
		void createInstance(ContextCreationInfo contextCreationInfo);
		void pickPhysicalDevice(ContextCreationInfo contextCreationInfo);
		void createDevice(ContextCreationInfo info);
		void release() const;
	};
}
