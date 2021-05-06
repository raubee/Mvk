#pragma once

#include "Device.hpp"

namespace mvk
{
	class RenderPass
	{
		Device* ptrDevice;

		vk::RenderPass renderPass;

	public:
		
		void create(Device* device, vk::Format colorFormat,
		          vk::Format depthFormat);

		void release() const;

		vk::RenderPass getRenderPass() const { return renderPass; }
	};
}
