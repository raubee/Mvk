#pragma once

#include "Vulkan.h"

namespace mvk
{
	class RenderPass
	{
		vk::Device device;
		vk::RenderPass renderPass;

	public:
		void init(vk::Device device, vk::Format colorFormat,
		          vk::Format depthFormat);
		void release() const;

		vk::RenderPass getRenderPass() const { return renderPass; }
	};
}
