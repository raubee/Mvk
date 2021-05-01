#pragma once

#include "Device.hpp"

namespace mvk
{
	class RenderPass
	{
		vk::RenderPass renderPass;

	public:
		
		void create(Device device, vk::Format colorFormat,
		          vk::Format depthFormat);

		void release(Device device);

		vk::RenderPass getRenderPass() const { return renderPass; }
	};
}
