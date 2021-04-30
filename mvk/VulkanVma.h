#pragma once

#define VMA_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include "../3rdParty/vk_mem_alloc.hpp"

namespace mvk
{
	namespace alloc
	{
		struct Buffer
		{
			vk::Buffer buffer;
			vma::Allocation allocation;
		};

		struct Image
		{
			vk::Image image;
			vma::Allocation allocation;
		};

		/** Allocator using vma allocator : https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator **/
		/** Hpp wrapper : https://github.com/malte-v/VulkanMemoryAllocator-Hpp **/
		/*** VMA Allocator ***/
		static vma::Allocator init(const vk::PhysicalDevice physicalDevice,
		                           const vk::Device device,
		                           const vk::Instance instance)
		{
			const vma::AllocatorCreateInfo allocatorCreateInfo = {
				.physicalDevice = physicalDevice,
				.device = device,
				.instance = instance,
				.vulkanApiVersion = VK_API_VERSION_1_2
			};

			return vma::createAllocator(allocatorCreateInfo);
		}

		static Buffer allocateMappedCpuToGpuBuffer(
			const vma::Allocator allocator,
			const vk::BufferCreateInfo
			bufferCreateInfo, void* data)
		{
			const vma::AllocationCreateInfo allocationCreateInfo = {
				.flags = vma::AllocationCreateFlagBits::eMapped,
				.usage = vma::MemoryUsage::eCpuToGpu
			};

			vma::AllocationInfo allocationInfo = {};

			const auto result = allocator.createBuffer(bufferCreateInfo,
			                                           allocationCreateInfo,
			                                           allocationInfo);

			memcpy(allocationInfo.pMappedData, data,
			       static_cast<size_t>(bufferCreateInfo.size));


			return {result.first, result.second};
		}

		static Buffer allocateCpuToGpuBuffer(
			const vma::Allocator allocator,
			const vk::BufferCreateInfo bufferCreateInfo)
		{
			const vma::AllocationCreateInfo allocationCreateInfo = {
				.usage = vma::MemoryUsage::eCpuToGpu,
			};

			vma::AllocationInfo allocationInfo = {};

			const auto result = allocator.createBuffer(bufferCreateInfo,
			                                           allocationCreateInfo,
			                                           allocationInfo);

			return {result.first, result.second};
		}

		static Buffer allocateGpuOnlyBuffer(
			const vma::Allocator allocator,
			const vk::BufferCreateInfo bufferCreateInfo)
		{
			const vma::AllocationCreateInfo allocationCreateInfo = {
				.usage = vma::MemoryUsage::eGpuOnly
			};

			vma::AllocationInfo allocationInfo = {};

			const auto result = allocator.createBuffer(bufferCreateInfo,
			                                           allocationCreateInfo,
			                                           allocationInfo);

			return {result.first, result.second};
		}

		static Image allocateGpuOnlyImage(
			const vma::Allocator allocator,
			const vk::ImageCreateInfo imageCreateInfo)
		{
			const vma::AllocationCreateInfo allocationCreateInfo = {
				.usage = vma::MemoryUsage::eGpuOnly
			};

			vma::AllocationInfo allocationInfo = {};

			const auto result = allocator.createImage(imageCreateInfo,
			                                          allocationCreateInfo,
			                                          allocationInfo);

			return {result.first, result.second};
		}

		static void deallocateBuffer(const vma::Allocator allocator,
		                             const Buffer buffer)
		{
			allocator.destroyBuffer(buffer.buffer, buffer.allocation);
		}

		static void deallocateImage(const vma::Allocator allocator,
		                            const Image image)
		{
			allocator.destroyImage(image.image, image.allocation);
		}

		static void mapDataToBuffer(const vma::Allocator allocator,
		                            const Buffer buffer,
		                            void* data,
		                            const size_t size
		)
		{
			void* mappedData;
			allocator.mapMemory(buffer.allocation, &mappedData);
			memcpy(mappedData, data, size);
			allocator.unmapMemory(buffer.allocation);
		}

		static Buffer allocateStagingTransferBuffer(
			const vma::Allocator allocator,
			void* data,
			const vk::DeviceSize
			size)
		{
			const vk::BufferCreateInfo bufferCreateInfo = {
				.size = static_cast<vk::DeviceSize>(size),
				.usage = vk::BufferUsageFlagBits::eTransferSrc,
				.sharingMode = vk::SharingMode::eExclusive
			};

			const auto buffer =
				allocateMappedCpuToGpuBuffer(allocator, bufferCreateInfo, data);

			return buffer;
		}

		static vk::CommandBuffer beginOneTimeSubmitCommands(
			const vk::Device device,
			const vk::CommandPool
			commandPool)
		{
			const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {
				.commandPool = commandPool,
				.commandBufferCount = 1
			};

			const auto commandBuffer = device.allocateCommandBuffers(
				                                  commandBufferAllocateInfo).
			                                  front();

			const vk::CommandBufferBeginInfo bufferBeginInfo = {
				.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
			};

			commandBuffer.begin(bufferBeginInfo);

			return commandBuffer;
		}

		static void endOneTimeSubmitCommands(const vk::Device device,
		                                     const vk::CommandPool commandPool,
		                                     const vk::CommandBuffer
		                                     commandBuffer,
		                                     const vk::Queue queue)
		{
			commandBuffer.end();

			vk::SubmitInfo submitInfo = {
				.commandBufferCount = 1,
				.pCommandBuffers = &commandBuffer
			};

			const auto result = queue.submit(1, &submitInfo, nullptr);
			queue.waitIdle();

			device.freeCommandBuffers(commandPool, 1, &commandBuffer);
		}

		static void copyCpuToGpuBuffer(const vk::Device device,
		                               const vk::CommandPool commandPool,
		                               const vk::Queue transferQueue,
		                               const Buffer src,
		                               const Buffer dst,
		                               const vk::DeviceSize size)
		{
			const auto commandBuffer =
				beginOneTimeSubmitCommands(device, commandPool);

			const vk::BufferCopy bufferCopy = {
				.size = size
			};

			commandBuffer.copyBuffer(src.buffer, dst.buffer, 1, &bufferCopy);

			endOneTimeSubmitCommands(device, commandPool, commandBuffer,
			                         transferQueue);
		}

		static void copyCpuBufferToGpuImage(const vk::Device device,
		                                    const vk::CommandPool commandPool,
		                                    const vk::Queue transferQueue,
		                                    const Buffer src,
		                                    const Image dst,
		                                    const vk::BufferImageCopy
		                                    bufferImageCopy)
		{
			const auto commandBuffer =
				beginOneTimeSubmitCommands(device, commandPool);


			commandBuffer.copyBufferToImage(src.buffer, dst.image,
			                                vk::ImageLayout::
			                                eTransferDstOptimal,
			                                1, &bufferImageCopy);

			endOneTimeSubmitCommands(device, commandPool, commandBuffer,
			                         transferQueue);
		}

		static void transitionImageLayout(
			const vk::Device device,
			const vk::CommandPool commandPool,
			const vk::Queue transferQueue,
			const vk::Image image, vk::Format format,
			const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout)
		{
			const auto commandBuffer =
				beginOneTimeSubmitCommands(device, commandPool);

			vk::ImageMemoryBarrier imageMemoryBarrier = {
				.oldLayout = oldLayout,
				.newLayout = newLayout,
				.image = image,
				.subresourceRange = {
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};


			if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
			{
				imageMemoryBarrier.subresourceRange.aspectMask =
					vk::ImageAspectFlagBits::eDepth;

				//TODO: Add Stencil aspect flag if needed 
				// if (hasStencilComponent(format)) {
				/*imageMemoryBarrier.subresourceRange.aspectMask |=
					vk::ImageAspectFlagBits::eStencil;*/
				//}
			}
			else
			{
				imageMemoryBarrier.subresourceRange.aspectMask =
					vk::ImageAspectFlagBits::eColor;
			}

			vk::PipelineStageFlags sourceStage;
			vk::PipelineStageFlags destinationStage;

			if (oldLayout == vk::ImageLayout::eUndefined)
			{
				imageMemoryBarrier.setSrcAccessMask(
					vk::AccessFlagBits::eNoneKHR);
				sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;

				if (newLayout == vk::ImageLayout::eTransferDstOptimal)
				{
					imageMemoryBarrier.setDstAccessMask(
						vk::AccessFlagBits::eTransferWrite);

					destinationStage = vk::PipelineStageFlagBits::eTransfer;
				}
				else if (newLayout ==
					vk::ImageLayout::eDepthStencilAttachmentOptimal)
				{
					imageMemoryBarrier.setDstAccessMask(
						vk::AccessFlagBits::eDepthStencilAttachmentRead |
						vk::AccessFlagBits::eDepthStencilAttachmentWrite);

					destinationStage =
						vk::PipelineStageFlagBits::eEarlyFragmentTests;
				}
			}
			else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
				newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
			{
				imageMemoryBarrier.setSrcAccessMask(
					vk::AccessFlagBits::eTransferWrite);
				imageMemoryBarrier.setDstAccessMask(
					vk::AccessFlagBits::eShaderRead);

				sourceStage = vk::PipelineStageFlagBits::eTransfer;
				destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
			}
			else
			{
				throw std::invalid_argument("Layout transition not supported!");
			}

			commandBuffer.pipelineBarrier(sourceStage, destinationStage,
			                              {},
			                              0, nullptr,
			                              0, nullptr,
			                              1, &imageMemoryBarrier);

			endOneTimeSubmitCommands(device, commandPool, commandBuffer,
			                         transferQueue);
		}

		static void mapMemory(const vma::Allocator allocator,
		                      const Buffer buffer, void* data)
		{
			allocator.mapMemory(buffer.allocation, &data);
		}

		static void unmapMemory(const vma::Allocator allocator,
		                        const Buffer buffer)
		{
			allocator.unmapMemory(buffer.allocation);
		}


		static Buffer createGpuBufferDst(const vma::Allocator allocator,
		                                 const vk::DeviceSize size,
		                                 const vk::BufferUsageFlagBits
		                                 usageFlag)
		{
			const vk::BufferCreateInfo bufferCreateInfo = {
				.size = static_cast<vk::DeviceSize>(size),
				.usage = vk::BufferUsageFlagBits::eTransferDst | usageFlag,
				.sharingMode = vk::SharingMode::eExclusive
			};

			const auto buffer =
				allocateGpuOnlyBuffer(allocator, bufferCreateInfo);

			return buffer;
		}

		static alloc::Buffer transferDataSetToGpuBuffer(
			const vma::Allocator allocator,
			const vk::Device device,
			const vk::CommandPool commandPool,
			const vk::Queue transferQueue,
			void* data,
			const vk::DeviceSize size,
			const vk::BufferUsageFlagBits usageFlag)
		{
			const auto stagingVertexBuffer =
				alloc::allocateStagingTransferBuffer(allocator, data, size);
			const auto indexBuffer = alloc::createGpuBufferDst(allocator, size,
			                                                   usageFlag);
			alloc::copyCpuToGpuBuffer(device, commandPool, transferQueue,
			                          stagingVertexBuffer, indexBuffer, size);
			alloc::deallocateBuffer(allocator, stagingVertexBuffer);

			return indexBuffer;
		}

		static alloc::Image transferImageDataToGpuImage(
			const vma::Allocator allocator,
			const vk::Device device,
			const vk::CommandPool commandPool,
			const vk::Queue transferQueue, 
			unsigned char* pixels,
			const uint32_t width,
			const uint32_t height,
			const vk::Format format)
		{
			const vk::DeviceSize imageSize = width * height * 4;
			const vk::BufferCreateInfo bufferCreateInfo = {
				.size = imageSize,
				.usage = vk::BufferUsageFlagBits::eTransferSrc
			};

			const auto stagingBuffer =
				alloc::allocateMappedCpuToGpuBuffer(allocator, bufferCreateInfo,
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

			const auto imageBuffer = alloc::allocateGpuOnlyImage(allocator,
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

			alloc::transitionImageLayout(device, commandPool, transferQueue,
				imageBuffer.image, format,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal);

			alloc::copyCpuBufferToGpuImage(device, commandPool, transferQueue,
				stagingBuffer, imageBuffer, bufferImageCopy);

			alloc::transitionImageLayout(device, commandPool, transferQueue,
				imageBuffer.image, format,
				vk::ImageLayout::eTransferDstOptimal,
				vk::ImageLayout::eShaderReadOnlyOptimal);

			alloc::deallocateBuffer(allocator, stagingBuffer);

			return imageBuffer;
		}
	}
};
