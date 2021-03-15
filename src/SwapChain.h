#pragma once

#include "Vulkan.h"
#include "SwapchainFrame.h"

namespace mvk
{
	struct SurfaceCapabilitiesKHRBatch
	{
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
	};

	class SwapChain
	{
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::CommandPool commandPool;
		vma::Allocator allocator;
		vk::Queue transferQueue;
		vk::SurfaceKHR surface;

		vk::SwapchainKHR swapchain;
		std::vector<vk::CommandBuffer> commandBuffers;
		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;
		std::vector<SwapchainFrame> swapchainFrames;

		AllocatedImage depthImage;
		vk::ImageView depthImageView;

		uint32_t swapchainSize;
		vk::Extent2D swapchainExtent;
		vk::Format swapchainFormat;
		vk::Format depthFormat;

		void createSwapchain();
		void createDepthImageView();
		SurfaceCapabilitiesKHRBatch getSwapchainCapabilities() const;
		vk::Format selectSwapchainFormat(
			std::vector<vk::SurfaceFormatKHR> formats) const;
		vk::PresentModeKHR selectSwapchainPresentMode() const;

	public:
		void init(vk::PhysicalDevice physicalDevice, vk::Device device,
		          vk::CommandPool commandPool, vma::Allocator allocator,
		          vk::Queue transferQueue, vk::SurfaceKHR surface);
		void createSwapchainFramesCommandBuffers();
		void createDescriptorPool();
		void createDescriptorSets(vk::DescriptorSetLayout descriptorSetLayout);
		void createSwapchainFrames(vk::RenderPass renderPass);
		void release();

		vk::SwapchainKHR getSwapchain() const { return swapchain; }

		size_t getSwapchainSwainSize() const { return swapchainSize; }
		vk::Extent2D getSwapchainExtent() const { return swapchainExtent; }
		vk::Format getSwapchainFormat() const { return swapchainFormat; }

		SwapchainFrame getSwapchainFrame(const int32_t index) const
		{
			return swapchainFrames.at(index);
		}

		std::vector<SwapchainFrame> getSwapchainFrames() const
		{
			return swapchainFrames;
		}

		vk::ImageView getDepthImageView() const
		{
			return depthImageView;
		}

		vk::Format getDepthFormat() const
		{
			return depthFormat;
		}
	};
}
