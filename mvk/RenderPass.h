#pragma once

#include "Device.hpp"

namespace mvk
{
	class RenderPass
	{
		Device* ptrDevice;

	public:

		vk::RenderPass renderPass;

		void create(Device* device, vk::Format colorFormat,
		            vk::Format depthFormat);

		void release() const;
	};
}
