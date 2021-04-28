#pragma once

#include "VulkanVma.h"

namespace mvk
{
	class Texture2D
	{
		unsigned char* pixels;
		uint32_t width;
		uint32_t height;
		vk::Format format;
		alloc::Image image;
		vk::ImageView imageView;
		vk::Sampler sampler;

		void createImageView(vk::Device device);
		void createSampler(vk::Device device);
		void cleanPixels() const;

	public:
		Texture2D(const char* path, vk::Format format);

		void init(vk::Device device, alloc::Image image);
		void release(vk::Device device, vma::Allocator allocator) const;

		vk::Sampler getSampler() const { return sampler; }
		vk::ImageView getImageView() const { return imageView; }

		unsigned char* getPixels() const { return pixels; }
		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		vk::Format getFormat() const { return format;  }
	};
}
