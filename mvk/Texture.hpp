#pragma once

#include "Device.hpp"

namespace mvk
{
	class Texture
	{
	protected:
		Device* ptrDevice;

		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t mipLevels = 0;

		vk::Format format = vk::Format::eUndefined;
		alloc::Image image;
		vk::ImageView imageView;
		vk::Sampler sampler;

		virtual void createImageView() = 0;
		virtual void createSampler() = 0;
		virtual void createDescriptorInfo() = 0;

		virtual alloc::Image copyDataToGpuImage(vk::Queue transferQueue,
		                                        const unsigned char*
		                                        pixels,
		                                        uint32_t width,
		                                        uint32_t height,
		                                        uint32_t mipLevels,
		                                        vk::Format format) = 0;

		void generateMipMaps(const vk::Queue transferQueue,
		                     const vk::Image image,
		                     const uint32_t baseArrayLayer) const
		{
			const auto commandBuffer = ptrDevice->beginOneTimeSubmitCommands();

			int mipWidth = width;
			int mipHeight = height;

			for (auto i = 1; i < mipLevels; i++)
			{
				vk::ImageMemoryBarrier imageMemoryBarrier{
					.srcAccessMask = vk::AccessFlagBits::eTransferWrite,
					.dstAccessMask = vk::AccessFlagBits::eTransferRead,
					.oldLayout = vk::ImageLayout::eTransferDstOptimal,
					.newLayout = vk::ImageLayout::eTransferSrcOptimal,
					.image = image,
					.subresourceRange{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.baseMipLevel = static_cast<uint32_t>(i - 1),
						.levelCount = 1,
						.baseArrayLayer = baseArrayLayer,
						.layerCount = 1
					}
				};

				commandBuffer.pipelineBarrier(
					vk::PipelineStageFlagBits::eTransfer,
					vk::PipelineStageFlagBits::
					eTransfer,
					vk::DependencyFlagBits::eByRegion,
					0, nullptr, 0, nullptr,
					1, &imageMemoryBarrier);

				const std::array<vk::Offset3D, 2> srcOffsets{
					vk::Offset3D{0, 0, 0},
					vk::Offset3D{mipWidth, mipHeight, 1}
				};

				const std::array<vk::Offset3D, 2> dstOffsets{
					vk::Offset3D{0, 0, 0},
					vk::Offset3D{
						mipWidth > 1 ? mipWidth / 2 : 1,
						mipHeight > 1 ? mipHeight / 2 : 1,
						1
					}
				};

				vk::ImageBlit imageBlit{
					.srcSubresource{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.mipLevel = static_cast<uint32_t>(i - 1),
						.baseArrayLayer = baseArrayLayer,
						.layerCount = 1
					},
					.srcOffsets = srcOffsets,

					.dstSubresource{
						.aspectMask = vk::ImageAspectFlagBits::eColor,
						.mipLevel = static_cast<uint32_t>(i),
						.baseArrayLayer = baseArrayLayer,
						.layerCount = 1
					},
					.dstOffsets = dstOffsets
				};


				commandBuffer.blitImage(image,
				                        vk::ImageLayout::eTransferSrcOptimal,
				                        image,
				                        vk::ImageLayout::eTransferDstOptimal, 1,
				                        &imageBlit,
				                        vk::Filter::eLinear);


				if (mipWidth > 1) mipWidth /= 2;
				if (mipHeight > 1) mipHeight /= 2;
			}

			vk::ImageMemoryBarrier imageMemoryBarrier{
				.srcAccessMask = vk::AccessFlagBits::eTransferRead,
				.dstAccessMask = vk::AccessFlagBits::eTransferWrite,
				.oldLayout = vk::ImageLayout::eTransferSrcOptimal,
				.newLayout = vk::ImageLayout::eTransferDstOptimal,
				.image = image,
				.subresourceRange{
					.aspectMask = vk::ImageAspectFlagBits::eColor,
					.baseMipLevel = 0,
					.levelCount = mipLevels - 1,
					.baseArrayLayer = baseArrayLayer,
					.layerCount = 1
				}
			};

			commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
			                              vk::PipelineStageFlagBits::
			                              eTransfer,
			                              vk::DependencyFlagBits::eByRegion,
			                              0, nullptr, 0, nullptr,
			                              1, &imageMemoryBarrier);

			ptrDevice->endOneTimeSubmitCommands(commandBuffer, transferQueue);
		}


	public:
		virtual vk::Sampler getSampler() const { return sampler; }
		virtual vk::ImageView getImageView() const { return imageView; }

		virtual uint32_t getWidth() const { return width; }
		virtual uint32_t getHeight() const { return height; }
		virtual vk::Format getFormat() const { return format; }

		virtual void release() const
		{
			ptrDevice->logicalDevice.destroySampler(sampler);
			ptrDevice->logicalDevice.destroyImageView(imageView);
			ptrDevice->destroyImage(image);
		}
	};
}