#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "SimpleViewer.h"
//#include "ObjViewer.h"

vk::SurfaceKHR getGlfwSurfaceKHR(mvk::Context context, GLFWwindow* window);
static void
framebufferResizeCallback(GLFWwindow* window, int width, int height);

int main()
{
	const auto appName = "Test";

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	const auto window = glfwCreateWindow(600, 600, appName, nullptr, nullptr);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	uint32_t glfwExtensionsCount = 0;
	const auto glfwExtensions = glfwGetRequiredInstanceExtensions(
		&glfwExtensionsCount);

	mvk::ContextCreationInfo createInfo;
	//createInfo.setQueueFamilyPreference(
	//	mvk::PreferredQueueFamilySettings::eGraphicTransferSeparated);

#if (NDEBUG)
	createInfo.addInstanceLayer("VK_LAYER_KHRONOS_validation");
	createInfo.addInstanceLayer("VK_LAYER_LUNARG_monitor");
	createInfo.addInstanceLayer("VK_LAYER_LUNARG_standard_validation");
#endif

	for (size_t i = 0; i < glfwExtensionsCount; i++)
	{
		createInfo.addInstanceExtension(glfwExtensions[i]);
	}

	createInfo.addDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	mvk::Context context;
	context.createInstance(createInfo);

	auto surface = getGlfwSurfaceKHR(context, window);

	context.pickPhysicalDevice(createInfo);
	context.createDevice(createInfo);

	//ObjViewer app;
	SimpleViewer app;
	glfwSetWindowUserPointer(window, &app);
	app.setup(context, surface);

	while (!glfwWindowShouldClose(window))
	{
		app.drawFrame();
		glfwPollEvents();
	}

	app.waitIdle();
	app.release();

	context.getInstance().destroySurfaceKHR(surface);
	context.release();

	glfwDestroyWindow(window);
	glfwTerminate();

	return EXIT_SUCCESS;
}


vk::SurfaceKHR getGlfwSurfaceKHR(const mvk::Context context, GLFWwindow* window)
{
	VkSurfaceKHR _surface;

	if (glfwCreateWindowSurface(
		static_cast<VkInstance>(context.getInstance()),
		window,
		nullptr,
		&_surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface khr");
	}

	const auto surface = vk::SurfaceKHR(_surface);
	return surface;
}

void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	/*const auto app =
		reinterpret_cast<ObjViewer*>(glfwGetWindowUserPointer(window));*/
	const auto app =
		reinterpret_cast<SimpleViewer*>(glfwGetWindowUserPointer(window));
	
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	app->setFramebufferDirty();
}
