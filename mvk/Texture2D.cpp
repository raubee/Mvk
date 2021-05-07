#include "Texture2D.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../3rdParty/stb_image.h"
#include <string>

using namespace mvk;

void Texture2D::loadFromFile(Device* device,
                             const vk::Queue transferQueue,
                             const char* path,
                             const vk::Format format)
{
	this->ptrDevice = device;

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
	this->image = device->transferImageDataToGpuImage(transferQueue, pixels,
	                                                  width, height, format);
	createImageView();
	createSampler();
	createDescriptorInfo();

	stbi_image_free(pixels);
}

void Texture2D::loadRaw(Device* device, const vk::Queue transferQueue,
                        const unsigned char* pixels, const int w,
                        const int h)
{
	this->ptrDevice = device;

	this->width = static_cast<uint32_t>(w);
	this->height = static_cast<uint32_t>(h);
	this->format = vk::Format::eR8G8B8A8Unorm;
	this->image = device->transferImageDataToGpuImage(transferQueue,
	                                                  pixels, width,
	                                                  height, format);

	createImageView();
	createSampler();
	createDescriptorInfo();
}

void Texture2D::createImageView()
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

	imageView = ptrDevice->logicalDevice.createImageView(imageViewCreateInfo);
}

void Texture2D::createSampler()
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

	sampler = ptrDevice->logicalDevice.createSampler(samplerCreateInfo);
}

void Texture2D::createDescriptorInfo()
{
	descriptorInfo = vk::DescriptorImageInfo{
		.sampler = getSampler(),
		.imageView = getImageView(),
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
	};
}

void Texture2D::release() const
{
	ptrDevice->logicalDevice.destroySampler(sampler);
	ptrDevice->logicalDevice.destroyImageView(imageView);
	ptrDevice->destroyImage(image);
}
