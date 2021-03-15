#include "ContextCreateInfo.h"

using namespace mvk;

ContextCreationInfo::ContextCreationInfo()
{
	applicationInfo = vk::ApplicationInfo{
		.pApplicationName = "mvk-app",
		.applicationVersion = 1,
		.pEngineName = "mvk-engine",
		.engineVersion = 1,
		.apiVersion = VK_API_VERSION_1_2,
	};
}

void ContextCreationInfo::addInstanceExtension(const char* extension)
{
	instanceExtensions.push_back(extension);
}

void ContextCreationInfo::addInstanceLayer(const char* layer)
{
	instanceLayers.push_back(layer);
}

void ContextCreationInfo::addDeviceExtension(const char* extension)
{
	deviceExtensions.push_back(extension);
}
