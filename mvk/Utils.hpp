#pragma once

#include "Vulkan.h"
#include <string>
#include <optional>
#include <iostream>

namespace mvk
{
	const struct QueueFamilies
	{
		std::optional<uint32_t> graphicQueue;
		std::optional<uint32_t> transferQueue;
	};

	enum class PreferredQueueFamilySettings
	{
		eGraphicsTransferTogether,
		eGraphicTransferSeparated,
	};

	static std::string formatQueueFlagsToString(const vk::QueueFlags flags)
	{
		std::stringstream output;
		if (flags & vk::QueueFlagBits::eGraphics) output << "Graphic |";
		if (flags & vk::QueueFlagBits::eTransfer) output << "Transfer |";
		if (flags & vk::QueueFlagBits::eCompute) output << "Compute |";
		if (flags & vk::QueueFlagBits::eProtected) output << "Protected |";
		if (flags & vk::QueueFlagBits::eSparseBinding) output <<
			"Sparse Binding";
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

	static bool isPhysicalDeviceSuitable(
		const vk::PhysicalDevice physicalDevice,
		const PreferredQueueFamilySettings settings,
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
		                queueFamilies.graphicQueue,
		                vk::QueueFlagBits::eProtected);

		findQueueFamily(queueFamilyProps, vk::QueueFlagBits::eTransfer,
		                queueFamilies.transferQueue, avoidFlag);

		if (queueFamilies.graphicQueue.has_value() &&
			queueFamilies.transferQueue.has_value())
		{
			return true;
		}

		return false;
	}
}
