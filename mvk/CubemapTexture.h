#pragma once

#include "Device.hpp"

namespace mvk
{
	class CubemapTexture
	{
		Device* ptrDevice;

		uint32_t width = 0;
		uint32_t height = 0;
		vk::Format format = vk::Format::eUndefined;
		alloc::Image image;
		vk::ImageView imageView;
		vk::Sampler sampler;

		alloc::Image copyDataToGpuImage(vk::Queue transferQueue,
		                                       const unsigned char* pixels,
		                                       uint32_t width,
		                                       uint32_t height,
		                                       vk::Format format) const;

		void createImageView();
		void createSampler();
		void createDescriptorInfo();

	public:

		vk::DescriptorImageInfo descriptorInfo;

		void loadFromSixFiles(Device* device,
		                  vk::Queue transferQueue,
		                  std::array<std::string, 6> texturePaths,
		                  vk::Format format);

		void release() const;

		vk::Sampler getSampler() const { return sampler; }
		vk::ImageView getImageView() const { return imageView; }

		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		vk::Format getFormat() const { return format; }
	};
}
