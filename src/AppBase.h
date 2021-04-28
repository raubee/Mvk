#pragma once
#include "Context.h"
#include "SwapChain.h"
#include "Scene.h"
#include "RenderPass.h"
#include "GraphicPipeline.h"

#include <chrono>

namespace mvk
{
	class AppBase
	{
	protected:
		Scene scene;
		SwapChain swapchain;
		vk::Device device;
		vma::Allocator allocator;
		RenderPass renderPass;
		std::vector<GraphicPipeline*> graphicPipelines;

		alloc::Buffer createVertexBufferObject(
			std::vector<Vertex> vertices) const;
		alloc::Buffer createIndexBufferObject(
			std::vector<uint16_t> indices) const;

		alloc::Image createTextureBufferObject(unsigned char* pixels,
		                                       uint32_t width,
		                                       uint32_t height,
		                                       vk::Format format);
		void loadTexture(Texture2D texture);
		void setupCommandBuffers();

		std::vector<vk::DescriptorSet> createDescriptorSets(
			GraphicPipeline graphicPipeline,
			uint32_t size);

	private:
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::SurfaceKHR surface;

		vk::CommandPool commandPool;
		vk::DescriptorPool descriptorPool;

		vk::Semaphore imageAvailableSemaphore;
		vk::Semaphore renderFinishedSemaphore;

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

		void createCommandPool(uint32_t queueFamily);
		void createSemaphores();
		void setupFrameCommandBuffer(int index,
		                             SwapchainFrame swapchainFrame);
		void updateSwapchain();
		void cleanupSwapchain();

		void update() const;

	public:
		AppBase(Context context, vk::SurfaceKHR surface);
		void release();
		void waitIdle() const;
		void drawFrame();
		void setSwapchainDirty();
	};
}
