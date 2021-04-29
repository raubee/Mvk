#include "Context.h"

#include <iostream>
#include <optional>

using namespace mvk;

const struct QueueFamilies
{
	std::optional<uint32_t> graphicQueue;
	std::optional<uint32_t> transferQueue;
};

static std::string formatQueueFlagsToString(const vk::QueueFlags flags)
{
	std::stringstream output;
	if (flags & vk::QueueFlagBits::eGraphics) output << "Graphic |";
	if (flags & vk::QueueFlagBits::eTransfer) output << "Transfer |";
	if (flags & vk::QueueFlagBits::eCompute) output << "Compute |";
	if (flags & vk::QueueFlagBits::eProtected) output << "Protected |";
	if (flags & vk::QueueFlagBits::eSparseBinding) output << "Sparse Binding";
	return output.str();
}

static void findQueueFamily(
	const std::vector<vk::QueueFamilyProperties> props,
	const vk::QueueFlagBits queueFlagBits,
	std::optional<uint32_t>& queueFamilyIndex,
	const vk::QueueFlagBits avoidQueueFlagBits)
{
	// Get the first index into queueFamilyProperties which supports queueFlagBits
	const auto result = std::find_if(
		props.begin(),
		props.end(),
		[queueFlagBits, avoidQueueFlagBits](auto const& qfp)
		{
			auto flag = qfp.queueFlags;

			if (flag & avoidQueueFlagBits)
				return false;

			return (flag & queueFlagBits) == queueFlagBits;
		});

	if (result != props.end())
	{
		const auto queueIndex =
			static_cast<uint32_t>(std::distance(props.begin(), result));

		if (queueIndex < props.size())
		{
			queueFamilyIndex = queueIndex;
		}
	}
}

static bool isPhysicalDeviceSuitable(const vk::PhysicalDevice physicalDevice,
                                     const PreferredQueueFamilySettings
                                     settings,
                                     QueueFamilies& queueFamilies)
{
	const auto queueFamilyProps = physicalDevice.getQueueFamilyProperties();

#if(NDEBUG)
	auto i = 0;
	for (const auto queueFamilyProp : queueFamilyProps)
	{
		std::cout
			<< "Queue family " << i << " :\n"
			<< "\tQueue count : " << queueFamilyProp.queueCount << "\n"
			<< "\tQueue flags : " << formatQueueFlagsToString(
				queueFamilyProp.queueFlags)
			<< std::endl;
		++i;
	}
#endif

	const auto avoidFlag = settings == PreferredQueueFamilySettings::
	                       eGraphicTransferSeparated
		                       ? vk::QueueFlagBits::eGraphics
		                       : vk::QueueFlagBits::eProtected;

	findQueueFamily(queueFamilyProps, vk::QueueFlagBits::eGraphics,
	                queueFamilies.graphicQueue, vk::QueueFlagBits::eProtected);

	findQueueFamily(queueFamilyProps, vk::QueueFlagBits::eTransfer,
	                queueFamilies.transferQueue, avoidFlag);

	if (queueFamilies.graphicQueue.has_value() &&
		queueFamilies.transferQueue.has_value())
	{
		return true;
	}

	return false;
}

void Context::createInstance(const ContextCreationInfo contextCreationInfo)
{
	const auto applicationInfo = contextCreationInfo.getApplicationInfo();

	vk::InstanceCreateInfo instanceCreateInfo = {
		.pApplicationInfo = &applicationInfo
	};

	auto extensions = contextCreationInfo.getInstanceExtensions();
	instanceCreateInfo.enabledExtensionCount =
		static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

#if (NDEBUG)
	auto layers = contextCreationInfo.getInstanceLayers();
	filterAvailableLayers(layers);
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
	instanceCreateInfo.ppEnabledLayerNames = layers.data();
#else
{
	instanceCreateInfo.enabledLayerCount = 0;
}
#endif

	instance = vk::createInstance(instanceCreateInfo);

#if (NDEBUG)
	/* print supported extensions */
	std::cout
		<< "Supported extensions: "
		<< std::endl;

	auto supportedExtensions = vk::
		enumerateInstanceExtensionProperties(nullptr);
	std::for_each(supportedExtensions.begin(),
	              supportedExtensions.end(),
	              [](auto const& e)
	              {
		              std::cout << e.extensionName << std::endl;
	              }
	);

	std::cout << std::endl;
#endif
}

void Context::filterAvailableLayers(std::vector<const char*>& layers)
{
	auto availableLayers = vk::enumerateInstanceLayerProperties();

#if (NDEBUG)
	/* print supported layers */
	std::cout
		<< "Available validation layers: "
		<< std::endl;

	std::for_each(availableLayers.begin(),
	              availableLayers.end(),
	              [](auto const& e)
	              {
		              std::cout << e.layerName << std::endl;
	              }
	);
	std::cout << std::endl;
#endif

	std::vector<const char*> retainLayers(0);

	for (auto layer : layers)
	{
		auto found = std::find_if(availableLayers.begin(),
		                          availableLayers.end(),
		                          [layer](auto const& available)
		                          {
			                          return std::strcmp(
					                          available.layerName, layer)
				                          == 0;
		                          });
		if (found != availableLayers.end())
		{
			retainLayers.push_back(layer);
		}
	}

#if (NDEBUG)
	/* print supported layers */
	std::cout
		<< "Selected Validation layers: "
		<< std::endl;

	std::for_each(retainLayers.begin(),
	              retainLayers.end(),
	              [](auto const& e)
	              {
		              std::cout << e << std::endl;
	              }
	);

	std::cout << std::endl;
#endif

	layers.clear();

	for (auto layer : retainLayers)
	{
		layers.push_back(layer);
	}
}

void Context::filterDeviceExtensions(std::vector<const char*>& extensions) const
{
	auto availableLayers = physicalDevice.enumerateDeviceExtensionProperties();

#if (NDEBUG)
	/* print supported extensions */
	std::cout
		<< "Available device extensions: "
		<< std::endl;

	std::for_each(availableLayers.begin(),
	              availableLayers.end(),
	              [](auto const& e)
	              {
		              std::cout << e.extensionName << std::endl;
	              }
	);
	std::cout << std::endl;
#endif

	std::vector<const char*> retainExtensions(0);

	for (auto layer : extensions)
	{
		auto found = std::find_if(availableLayers.begin(),
		                          availableLayers.end(),
		                          [layer](auto const& available)
		                          {
			                          return std::strcmp(
					                          available.extensionName, layer)
				                          == 0;
		                          });

		if (found != availableLayers.end())
		{
			retainExtensions.push_back(layer);
		}
	}

#if (NDEBUG)
	/* print supported extensions */
	std::cout
		<< "Selected device extensions: "
		<< std::endl;

	std::for_each(retainExtensions.begin(),
	              retainExtensions.end(),
	              [](auto const& e)
	              {
		              std::cout << e << std::endl;
	              }
	);

	std::cout << std::endl;
#endif

	extensions.clear();

	for (auto layer : retainExtensions)
	{
		extensions.push_back(layer);
	}
}

void Context::pickPhysicalDevice(
	const ContextCreationInfo contextCreationInfo)
{
	auto physicalDevices = instance.enumeratePhysicalDevices();

	for (const auto physicalDevice : physicalDevices)
	{
		QueueFamilies queueFamilies;
		if (isPhysicalDeviceSuitable(physicalDevice,
		                             contextCreationInfo.
		                             getQueueFamilyPreference(),
		                             queueFamilies))
		{
			this->physicalDevice = physicalDevice;
			graphicsQueueFamilyIndex = queueFamilies.graphicQueue.value();
			transferQueueFamilyIndex = queueFamilies.transferQueue.value();
			deviceFeatures = physicalDevice.getFeatures();

#if(NDEBUG)
			std::cout
				<< "Selected physical device: "
				<< physicalDevice.getProperties().deviceName
				<< std::endl;

			std::cout
				<< "Selected graphics queue family index: "
				<< graphicsQueueFamilyIndex
				<< std::endl;

			std::cout
				<< "Selected transfer queue family index: "
				<< transferQueueFamilyIndex
				<< std::endl;

			std::cout << std::endl;
#endif

			return;
		}
	}

	throw std::runtime_error("Failed: Can't found a suitable physical device");
}

void Context::createDevice(const ContextCreationInfo info)
{
	std::vector<vk::DeviceQueueCreateInfo> deviceQueues;

	if (info.getQueueFamilyPreference() == PreferredQueueFamilySettings::
		eGraphicsTransferTogether)
	{
		const std::array<const float, 3> queuePriority = {0.0f, 0.0f, 0.0f};

		const vk::DeviceQueueCreateInfo deviceGraphicQueueCreateInfo
		{
			.queueFamilyIndex = graphicsQueueFamilyIndex,
			.queueCount = 3,
			.pQueuePriorities = queuePriority.data()
		};

		deviceQueues.push_back(deviceGraphicQueueCreateInfo);
	}
	else
	{
		const std::array<const float, 2> queuePriority = {0.0f, 0.0f};

		const vk::DeviceQueueCreateInfo deviceGraphicQueueCreateInfo
		{
			.queueFamilyIndex = graphicsQueueFamilyIndex,
			.queueCount = 2,
			.pQueuePriorities = queuePriority.data()
		};

		const auto queuePriorityT = 0.0f;

		const vk::DeviceQueueCreateInfo deviceTransferQueueCreateInfo
		{
			.queueFamilyIndex = transferQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriorityT
		};

		deviceQueues.push_back(deviceGraphicQueueCreateInfo);
		deviceQueues.push_back(deviceTransferQueueCreateInfo);
	}

	auto deviceExtensions = info.getDeviceExtensions();
	filterDeviceExtensions(deviceExtensions);

#if (NDEBUG)
	/* List device validation layers */
	auto deviceLayers = physicalDevice.enumerateDeviceLayerProperties();
	std::cout
		<< "Supported devices layers: "
		<< std::endl;
	for (auto layer : deviceLayers)
	{
		std::cout << layer.layerName << std::endl;
	}
	std::cout << std::endl;
#endif

	const vk::DeviceCreateInfo deviceCreateInfo{
		.queueCreateInfoCount = static_cast<uint32_t>(deviceQueues.size()),
		.pQueueCreateInfos = deviceQueues.data(),
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data(),
		.pEnabledFeatures = &deviceFeatures
	};

	device = physicalDevice.createDevice(deviceCreateInfo);

	if (info.getQueueFamilyPreference() == PreferredQueueFamilySettings::
		eGraphicsTransferTogether)
	{
		graphicsQueue = device.getQueue(getGraphicsQueueFamilyIndex(), 0);
		presentQueue = device.getQueue(getGraphicsQueueFamilyIndex(), 1);
		transferQueue = device.getQueue(getGraphicsQueueFamilyIndex(), 2);
	}
	else
	{
		graphicsQueue = device.getQueue(getGraphicsQueueFamilyIndex(), 0);
		presentQueue = device.getQueue(getGraphicsQueueFamilyIndex(), 1);
		transferQueue = device.getQueue(getTransferQueueFamilyIndex(), 0);
	}

	std::cout << "Logical device created!" << std::endl;
}

void Context::release() const
{
	device.destroy();
	instance.destroy();
}
