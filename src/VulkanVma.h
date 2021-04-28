#pragma once

#include "Context.h"

namespace mvk::alloc
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
	static vma::Allocator init(const Context context)
	{
		vma::AllocatorCreateInfo allocatorCreateInfo = {
			.physicalDevice = context.getPhysicalDevice(),
			.device = context.getDevice(),
			.instance = context.getInstance(),
			.vulkanApiVersion = VK_API_VERSION_1_2
		};

		return vma::createAllocator(allocatorCreateInfo);
	}

	static Buffer allocateMappedCpuToGpuBuffer(const vma::Allocator allocator,
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

	static vk::CommandBuffer beginOneTimeSubmitCommands(const vk::Device device,
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
	                                     const vk::CommandBuffer commandBuffer,
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
};
