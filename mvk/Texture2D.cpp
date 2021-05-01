#include "Texture2D.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../3rdParty/stb_image.h"
#include <string>

using namespace mvk;

void Texture2D::loadFromFile(const Device device,
                             const vk::Queue transferQueue,
                             const char* path,
                             const vk::Format format)
{
	int w, h, c;
	const auto pixels = stbi_load(path, &w, &h, &c,
	                              STBI_rgb_alpha);

	if (!pixels)
	{
		throw std::runtime_error(std::string("Failed to load texture: ") +
			path);
	}

	this->width = static_cast<uint32_t>(w);
	this->height = static_cast<uint32_t>(h);
	this->format = format;
	this->image = device.transferImageDataToGpuImage(transferQueue, pixels,
	                                                 width, height, format);
	createImageView(device);
	createSampler(device);

	stbi_image_free(pixels);
}


void Texture2D::createImageView(const Device device)
{
	const vk::ImageViewCreateInfo imageViewCreateInfo = {
		.image = image.image,
		.viewType = vk::ImageViewType::e2D,
		.format = format,
		.components = vk::ComponentSwizzle::eIdentity,
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	imageView = vk::Device(device).createImageView(imageViewCreateInfo);
}

void Texture2D::createSampler(const Device device)
{
	const vk::SamplerCreateInfo samplerCreateInfo = {
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f,
		//TODO: .anisotropyEnable = vk::Bool32(true),
		// .maxAnisotropy = Get from physical device support anisotropy
		.compareEnable = vk::Bool32(false),
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.0f,
		.maxLod = 0.0f,
		.borderColor = vk::BorderColor::eIntOpaqueBlack,
		.unnormalizedCoordinates = vk::Bool32(false),
	};

	sampler = vk::Device(device).createSampler(samplerCreateInfo);
}

void Texture2D::release(const Device device) const
{
	vk::Device(device).destroySampler(sampler);
	vk::Device(device).destroyImageView(imageView);
	device.destroyImage(image);
}
