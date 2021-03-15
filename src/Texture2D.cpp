#include "Texture2D.h"
#define STB_IMAGE_IMPLEMENTATION
#include "3rdParty/stb_image.h"
#include <string>

using namespace mvk;

void Texture2D::load(const vma::Allocator allocator, const vk::Device device,
                     const vk::CommandPool commandPool,
                     const vk::Queue transferQueue, const char* filename)
{
	this->device = device;
	this->allocator = allocator;
	this->commandPool = commandPool;
	this->queue = transferQueue;

	loadImage(filename);
	createImageView();
	createSampler();
}

void Texture2D::release() const
{
	device.destroySampler(sampler);
	device.destroyImageView(imageView);
	deallocateImage(allocator, image);
}

void Texture2D::loadImage(const char* filename)
{
	int texWidth, texHeight, texChannels;
	const auto pixels = stbi_load(filename, &texWidth, &texHeight, &texChannels,
	                              STBI_rgb_alpha);
	const vk::DeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		throw std::runtime_error(std::string("Failed to load texture: ") +
			filename);
	}

	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = imageSize,
		.usage = vk::BufferUsageFlagBits::eTransferSrc
	};

	const auto stagingBuffer = allocateMappedCpuToGpuBuffer(allocator,
	                                                        bufferCreateInfo,
	                                                        pixels);
	const vk::Extent3D imageExtent = {
		.width = static_cast<uint32_t>(texWidth),
		.height = static_cast<uint32_t>(texHeight),
		.depth = 1
	};

	format = vk::Format::eR8G8B8A8Srgb;

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

	image = allocateGpuOnlyImage(allocator, imageCreateInfo);

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

	transitionImageLayout(allocator, device, commandPool, queue,
	                      image.image, format, vk::ImageLayout::eUndefined,
	                      vk::ImageLayout::eTransferDstOptimal);

	copyCpuBufferToGpuImage(allocator, device, commandPool, queue,
	                        stagingBuffer, image, bufferImageCopy);

	transitionImageLayout(allocator, device, commandPool, queue,
	                      image.image, format,
	                      vk::ImageLayout::eTransferDstOptimal,
	                      vk::ImageLayout::eShaderReadOnlyOptimal);

	stbi_image_free(pixels);
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

	imageView = device.createImageView(imageViewCreateInfo);
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

	sampler = device.createSampler(samplerCreateInfo);
}
