#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "Utils.hpp"
#include "SwapChain.h"
#include "Vertex.h"
#include "Scene.h"
#include "RenderPass.h"

#include <chrono>

namespace mvk
{
	struct AppInfo
	{
		const char* appName = "App";
		int width = 600;
		int height = 600;
		bool fullscreen = false;
	};

	class AppBase
	{
	protected:

		Device device;
		SwapChain swapchain;
		RenderPass renderPass;
		Scene scene;

		vk::Queue graphicsQueue;
		vk::Queue transferQueue;
		vk::Queue presentQueue;

		PreferredQueueFamilySettings preferredQueueFamilySetting;

		uint32_t graphicsQueueFamilyIndex;
		uint32_t transferQueueFamilyIndex;
		
	private:
		const char* appName;
		int width;
		int height;

		vk::ApplicationInfo applicationInfo;

		GLFWwindow* window;
		std::vector<const char*> glfwExtensions;

		vk::SurfaceKHR surface;

		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;

		vk::Semaphore imageAvailableSemaphore;
		vk::Semaphore renderFinishedSemaphore;

		bool needResize;

		std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

		void setupWindow(bool fullscreen);
		static void framebufferResizeCallback(GLFWwindow* window, int width,
		                                      int height);

		void filterAvailableLayers(std::vector<const char*>& layers);

		void createInstance();
		void createSurfaceKHR();
		void pickPhysicalDevice();
		void createDevice();
		void createQueues();
		void createSwapchain();
		void createSwapchainFrames();
		void updateSwapchain();
		void createRenderPass();
		void createSemaphores();
		void setupScene();

		virtual void buildCommandBuffers();

		void waitIdle() const;
		void drawFrame();
		void update() const;
		void setSwapchainDirty();

	public:
		AppBase(AppInfo info);
		virtual ~AppBase();
		void run();
	};
}
