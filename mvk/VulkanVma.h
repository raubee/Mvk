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
			bufferCreateInfo, const void* data)
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
			const vk::DeviceSize size)
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
	}
};
