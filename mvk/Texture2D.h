#pragma once

#include "Device.hpp"

namespace mvk
{
	class Texture2D
	{
		Device* ptrDevice;

		uint32_t width = 0;
		uint32_t height = 0;
		vk::Format format = vk::Format::eUndefined;
		alloc::Image image;
		vk::ImageView imageView;
		vk::Sampler sampler;

		void createImageView();
		void createSampler();
		void createDescriptorInfo();

	public:

		vk::DescriptorImageInfo descriptorInfo;

		void loadRaw(Device* device, vk::Queue transferQueue,
		             const unsigned char* pixels, int w, int h);

		void loadFromFile(Device* device,
		                  vk::Queue transferQueue,
		                  const char* path,
		                  vk::Format format);

		void release() const;

		vk::Sampler getSampler() const { return sampler; }
		vk::ImageView getImageView() const { return imageView; }

		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		vk::Format getFormat() const { return format; }

		inline static Texture2D* empty;
	};
}
