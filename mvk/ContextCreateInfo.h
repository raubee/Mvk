#pragma once

#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan.hpp>

namespace mvk
{
	enum class PreferredQueueFamilySettings
	{
		eGraphicsTransferTogether,
		eGraphicTransferSeparated,
	};

	class ContextCreationInfo
	{
		vk::ApplicationInfo applicationInfo;

		std::vector<const char*> instanceExtensions;
		std::vector<const char*> instanceLayers;
		std::vector<const char*> deviceExtensions;

		PreferredQueueFamilySettings preferredQueueFamilySetting;

	public:
		ContextCreationInfo();

		void addInstanceExtension(const char* extension);
		void addInstanceLayer(const char* layer);

		void addDeviceExtension(const char* extension);

		void setQueueFamilyPreference(
			const PreferredQueueFamilySettings settings)
		{
			preferredQueueFamilySetting = settings;
		}

		PreferredQueueFamilySettings getQueueFamilyPreference() const
		{
			return preferredQueueFamilySetting;
		}

		vk::ApplicationInfo getApplicationInfo() const
		{
			return applicationInfo;
		}

		std::vector<const char*> getInstanceExtensions() const
		{
			return instanceExtensions;
		}

		std::vector<const char*> getInstanceLayers() const
		{
			return instanceLayers;
		}

		std::vector<const char*> getDeviceExtensions() const
		{
			return deviceExtensions;
		};
	};
}
