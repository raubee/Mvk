#pragma once

#include "Context.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "SwapChain.h"
#include "Vertex.h"
#include "Scene.h"
#include "RenderPass.h"

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

		alloc::Buffer createVertexBufferObject(
			std::vector<Vertex> vertices) const;
		alloc::Buffer createIndexBufferObject(
			std::vector<uint16_t> indices) const;

		alloc::Image createTextureBufferObject(unsigned char* pixels,
		                                       uint32_t width,
		                                       uint32_t height,
		                                       vk::Format format) const;

		void setupCommandBuffers();

	private:

		Context context;
		GLFWwindow* window;

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

		void update();

	public:

		AppBase();

		void release();
		void waitIdle() const;
		void drawFrame();
		void setSwapchainDirty();

		void run();
		void terminate();

		vk::SurfaceKHR getGlfwSurfaceKHR(const mvk::Context context,
		                                 GLFWwindow* window);
	};
}
