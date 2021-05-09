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
	this->image = copyDataToGpuImage(transferQueue, pixels,
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
	this->image = copyDataToGpuImage(transferQueue, pixels,
	                                 width, height, format);

	createImageView();
	createSampler();
	createDescriptorInfo();
}

alloc::Image Texture2D::copyDataToGpuImage(const vk::Queue transferQueue,
                                           const unsigned char* pixels,
                                           const uint32_t width,
                                           const uint32_t height,
                                           const vk::Format format) const
{
	const vk::DeviceSize imageSize = width * height * 4;
	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = imageSize,
		.usage = vk::BufferUsageFlagBits::eTransferSrc
	};

	const auto stagingBuffer =
		alloc::allocateMappedCpuToGpuBuffer(ptrDevice->allocator,
		                                    bufferCreateInfo,
		                                    pixels);

	const vk::Extent3D imageExtent = {
		.width = width,
		.height = height,
		.depth = 1
	};

	const vk::ImageCreateInfo imageCreateInfo = {
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = imageExtent,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = vk::ImageTiling::eOptimal,
		.usage = vk::ImageUsageFlagBits::eTransferDst |
		vk::ImageUsageFlagBits::eSampled,
		.sharingMode = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::eUndefined,
	};

	const auto imageBuffer = alloc::allocateGpuOnlyImage(ptrDevice->allocator,
	                                                     imageCreateInfo);

	const vk::BufferImageCopy bufferImageCopy = {
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
		.imageOffset = {0, 0},
		.imageExtent = imageExtent,
	};

	const vk::ImageSubresourceRange subresourceRange{
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1
	};

	ptrDevice->transitionImageLayout(transferQueue, imageBuffer.image,
	                                 vk::ImageLayout::eUndefined,
	                                 vk::ImageLayout::eTransferDstOptimal,
	                                 subresourceRange);

	ptrDevice->copyCpuBufferToGpuImage(transferQueue, stagingBuffer,
	                                   imageBuffer,
	                                   bufferImageCopy);

	ptrDevice->transitionImageLayout(transferQueue, imageBuffer.image,
	                                 vk::ImageLayout::eTransferDstOptimal,
	                                 vk::ImageLayout::eShaderReadOnlyOptimal,
	                                 subresourceRange);

	alloc::deallocateBuffer(ptrDevice->allocator, stagingBuffer);

	return imageBuffer;
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
