#pragma once
#include "Context.h"
#include "SwapChain.h"
#include "GraphicPipeline.h"
#include "Scene.h"

#include <chrono>

namespace mvk
{
	class AppBase
	{
	protected:
		Scene scene;

	private:
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		vk::SurfaceKHR surface;

		vk::CommandPool commandPool;

		vk::Semaphore imageAvailableSemaphore;
		vk::Semaphore renderFinishedSemaphore;

		SwapChain swapchain;
		RenderPass renderPass;
		GraphicPipeline graphicPipeline;

		vma::Allocator allocator;

		vk::Queue graphicQueue;
		vk::Queue transferQueue;
		vk::Queue presentQueue;

		bool needResize;
		std::chrono::time_point<std::chrono::high_resolution_clock> startTime;


		const std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		bool checkSwapchainSupport();
		void createSwapchain();
		void createRenderPass();
		void createSwapchainFrames();
		void createGraphicPipeline();
		void createDescriptorSets();
		void createCommandPool(uint32_t graphicsQueueFamilyIndex);
		void setupSwapchainFramesCommandBuffers();

		void createSemaphores();

		void updateUniformBuffer(SwapchainFrame swapchainFrame);

		void updateSwapchain();
		void cleanupSwapchain();

	public:
		virtual void setup(Context context, vk::SurfaceKHR surface);
		void release();
		void waitIdle() const;
		void drawFrame();
		void setFramebufferDirty();
	};
}
