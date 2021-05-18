#pragma once

#include "Texture.hpp"

namespace mvk
{
	class Texture2D : public Texture
	{
		alloc::Image copyDataToGpuImage(vk::Queue transferQueue,
		                                const unsigned char* pixels,
		                                uint32_t width,
		                                uint32_t height,
		                                uint32_t mipLevels,
		                                vk::Format format) override;

		void createImageView() override;
		void createSampler() override;
		void createDescriptorInfo() override;

	public:

		vk::DescriptorImageInfo descriptorInfo;

		void loadRaw(Device* device, vk::Queue transferQueue,
		             const unsigned char* pixels, int w, int h);

		void loadFromFile(Device* device,
		                  vk::Queue transferQueue,
		                  const char* path,
		                  vk::Format format);

		inline static Texture2D* empty;
	};
}
