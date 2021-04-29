#pragma once

#include "Vulkan.h"

namespace mvk
{
	class RenderPass
	{
		vk::RenderPass renderPass;

	public:
		
		void create(vk::Device device, vk::Format colorFormat,
		          vk::Format depthFormat);

		void release(vk::Device device) const;

		vk::RenderPass getRenderPass() const { return renderPass; }
	};
}
