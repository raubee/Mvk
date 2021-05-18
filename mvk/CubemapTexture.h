#pragma once

#include "Texture.hpp"

namespace mvk
{
	class CubemapTexture : public Texture
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

		void loadFromSixFiles(Device* device,
		                      vk::Queue transferQueue,
		                      std::array<std::string, 6> texturePaths,
		                      vk::Format format);
	};
}
