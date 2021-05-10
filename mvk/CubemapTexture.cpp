#include "CubemapTexture.h"
#include "../3rdParty/stb_image.h"
#include <string>

using namespace mvk;

void CubemapTexture::loadFromSixFiles(Device* device,
                                      const vk::Queue transferQueue,
                                      const std::array<std::string, 6>
                                      texturePaths,
                                      const vk::Format format)
{
	this->ptrDevice = device;

	const auto nbTextures = 6;
	vk::DeviceSize imageSize = 0;

	unsigned char* pixels = nullptr;

	for (auto i = 0; i < nbTextures; i++)
	{
		int w, h, c;
		const auto p = stbi_load(texturePaths[i].c_str(), &w, &h, &c,
		                         STBI_rgb_alpha);
		if (i == 0)
		{
			this->width = static_cast<uint32_t>(w);
			this->height = static_cast<uint32_t>(h);
			imageSize = width * height * 4;
			pixels = new unsigned char[imageSize * nbTextures];
		}

		if (!p)
		{
			throw std::
				runtime_error(std::string("Failed to load texture: ") +
					texturePaths[i]);
		}

		memcpy(&pixels[imageSize * i], p, imageSize);

		stbi_image_free(p);
	}

	this->format = format;
	this->image = copyDataToGpuImage(transferQueue, pixels,
	                                 width, height, format);
	createImageView();
	createSampler();
	createDescriptorInfo();
}

alloc::Image CubemapTexture::copyDataToGpuImage(
	const vk::Queue transferQueue,
	const unsigned char* pixels,
	const uint32_t width,
	const uint32_t height,
	const vk::Format format) const
{
	const vk::DeviceSize imageSize = width * height * 4 * 6;
	const vk::BufferCreateInfo bufferCreateInfo{
		.size = imageSize,
		.usage = vk::BufferUsageFlagBits::eTransferSrc
	};

	const auto stagingBuffer =
		alloc::allocateMappedCpuToGpuBuffer(ptrDevice->allocator,
		                                    bufferCreateInfo,
		                                    pixels);

	const vk::Extent3D imageExtent{
		.width = width,
		.height = height,
		.depth = 1
	};

	const vk::ImageCreateInfo imageCreateInfo{
		.flags = vk::ImageCreateFlagBits::eCubeCompatible,
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = imageExtent,
		.mipLevels = 1,
		.arrayLayers = 6,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = vk::ImageTiling::eOptimal,
		.usage = vk::ImageUsageFlagBits::eTransferDst |
		vk::ImageUsageFlagBits::eSampled,
		.sharingMode = vk::SharingMode::eExclusive,
		.initialLayout = vk::ImageLayout::eUndefined,
	};

	const auto imageBuffer = alloc::allocateGpuOnlyImage(ptrDevice->allocator,
	                                                     imageCreateInfo);

	vk::BufferImageCopy bufferImageCopy{
		.bufferOffset = 0,
		.bufferRowLength = width,
		.bufferImageHeight = height,
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
		.layerCount = 6
	};

	for (auto i = 0; i < 6; i++)
	{
		bufferImageCopy.imageSubresource.baseArrayLayer = i;
		bufferImageCopy.bufferOffset = i * width * height * 4;

		ptrDevice->transitionImageLayout(transferQueue, imageBuffer.image,
		                                 vk::ImageLayout::eUndefined,
		                                 vk::ImageLayout::eTransferDstOptimal,
		                                 subresourceRange);

		ptrDevice->copyCpuBufferToGpuImage(transferQueue, stagingBuffer,
		                                   imageBuffer,
		                                   bufferImageCopy);

		ptrDevice->transitionImageLayout(transferQueue, imageBuffer.image,
		                                 vk::ImageLayout::eTransferDstOptimal,
		                                 vk::ImageLayout::
		                                 eShaderReadOnlyOptimal,
		                                 subresourceRange);
	}

	alloc::deallocateBuffer(ptrDevice->allocator, stagingBuffer);

	return imageBuffer;
}

void CubemapTexture::createImageView()
{
	const vk::ImageViewCreateInfo imageViewCreateInfo{
		.image = image.image,
		.viewType = vk::ImageViewType::eCube,
		.format = format,
		.components = vk::ComponentSwizzle::eIdentity,
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 6
		}
	};

	imageView = ptrDevice->logicalDevice.createImageView(imageViewCreateInfo);
}

void CubemapTexture::createSampler()
{
	const vk::SamplerCreateInfo samplerCreateInfo{
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

void CubemapTexture::createDescriptorInfo()
{
	descriptorInfo = vk::DescriptorImageInfo{
		.sampler = getSampler(),
		.imageView = getImageView(),
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
	};
}

void CubemapTexture::release() const
{
	ptrDevice->logicalDevice.destroySampler(sampler);
	ptrDevice->logicalDevice.destroyImageView(imageView);
	ptrDevice->destroyImage(image);
}
