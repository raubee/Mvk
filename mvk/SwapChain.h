#pragma once

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
		vk::SwapchainKHR swapchain;
		std::vector<vk::CommandBuffer> commandBuffers;
		std::vector<SwapchainFrame> swapchainFrames;

		alloc::Image depthImage;
		vk::ImageView depthImageView;

		uint32_t size = 0;
		vk::Extent2D extent;
		vk::Format frameFormat = vk::Format::eUndefined;
		vk::Format depthFormat = vk::Format::eUndefined;

		void createDepthImageView(vk::Device device,
		                          vma::Allocator allocator,
		                          vk::CommandPool commandPool,
		                          vk::Queue transferQueue);

		SurfaceCapabilitiesKHRBatch getSwapchainCapabilities(
			vk::PhysicalDevice physicalDevice,
			vk::SurfaceKHR surface) const;

		vk::Format selectSwapchainFormat(
			std::vector<vk::SurfaceFormatKHR> formats) const;

		vk::PresentModeKHR selectSwapchainPresentMode(
			vk::PhysicalDevice physicalDevice,
			vk::SurfaceKHR surface) const;

	public:
		void create(vk::PhysicalDevice physicalDevice,
		            vk::Device device,
		            vma::Allocator allocator,
		            vk::CommandPool commandPool,
		            vk::Queue transferQueue,
		            vk::SurfaceKHR surface);

		void createSwapChainKHR(vk::PhysicalDevice physicalDevice,
		                        vk::Device device,
		                        vk::SurfaceKHR surface);

		void createCommandBuffers(vk::Device device,
		                          vk::CommandPool commandPool);

		void createSwapchainFrames(vk::Device device,
		                           vk::RenderPass renderPass);

		void release(vk::Device device, vma::Allocator allocator);

		vk::SwapchainKHR getSwapchain() const { return swapchain; }

		size_t getSwapchainSwainSize() const { return size; }
		vk::Extent2D getSwapchainExtent() const { return extent; }
		vk::Format getSwapchainFormat() const { return frameFormat; }

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
