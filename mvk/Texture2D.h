#pragma once

#include "VulkanVma.h"

namespace mvk
{
	class Texture2D
	{
		uint32_t width = 0;
		uint32_t height = 0;
		vk::Format format = vk::Format::eUndefined;
		alloc::Image image;
		vk::ImageView imageView;
		vk::Sampler sampler;

		void createImageView(vk::Device device);
		void createSampler(vk::Device device);

	public:
		void loadFromFile(vma::Allocator allocator,
		                  vk::Device device,
		                  vk::CommandPool commandPool,
		                  vk::Queue transferQueue,
		                  const char* path,
		                  vk::Format format);

		void release(vk::Device device, vma::Allocator allocator) const;

		vk::Sampler getSampler() const { return sampler; }
		vk::ImageView getImageView() const { return imageView; }

		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		vk::Format getFormat() const { return format; }
	};
}
