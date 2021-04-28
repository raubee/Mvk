#include "Texture2D.h"
#define STB_IMAGE_IMPLEMENTATION
#include "3rdParty/stb_image.h"
#include <string>

using namespace mvk;

Texture2D::Texture2D(const char* path, const vk::Format format)
{
	int w, h, c;
	pixels = stbi_load(path, &w, &h, &c,
	                   STBI_rgb_alpha);

	if (!pixels)
	{
		throw std::runtime_error(std::string("Failed to load texture: ") +
			path);
	}

	width = static_cast<uint32_t>(w);
	height = static_cast<uint32_t>(h);
	this->format = format;
}

void Texture2D::init(const vk::Device device, const alloc::Image image)
{
	this->image = image;

	createImageView(device);
	createSampler(device);
	cleanPixels();
}

void Texture2D::createImageView(const vk::Device device)
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

	imageView = device.createImageView(imageViewCreateInfo);
}

void Texture2D::createSampler(const vk::Device device)
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

	sampler = device.createSampler(samplerCreateInfo);
}

void Texture2D::release(const vk::Device device,
                        const vma::Allocator allocator) const
{
	cleanPixels();

	device.destroySampler(sampler);
	device.destroyImageView(imageView);
	deallocateImage(allocator, image);
}

void Texture2D::cleanPixels() const
{
	if (pixels != nullptr)
		stbi_image_free(pixels);
}
