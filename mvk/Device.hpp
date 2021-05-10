#pragma once

#include "VulkanVma.h"
#include "Utils.hpp"

namespace mvk
{
	class Device
	{
	public:
		vk::Device logicalDevice;
		vma::Allocator allocator;
		vk::PhysicalDevice physicalDevice;
		vk::CommandPool commandPool;
		uint32_t graphicsQueueFamilyIndex;

		vk::SampleCountFlagBits multiSampling;

		void filterDeviceExtensions(std::vector<const char*>& extensions) const
		{
			auto availableLayers = physicalDevice.
				enumerateDeviceExtensionProperties();

#if (NDEBUG)
			/* print supported extensions */
			std::cout
				<< "Available device extensions: "
				<< std::endl;

			std::for_each(availableLayers.begin(),
			              availableLayers.end(),
			              [](auto const& e)
			              {
				              std::cout << e.extensionName << std::endl;
			              }
			);
			std::cout << std::endl;
#endif

			std::vector<const char*> retainExtensions(0);

			for (auto layer : extensions)
			{
				auto found = std::find_if(availableLayers.begin(),
				                          availableLayers.end(),
				                          [layer](auto const& available)
				                          {
					                          return std::strcmp(
							                          available.extensionName,
							                          layer)
						                          == 0;
				                          });

				if (found != availableLayers.end())
				{
					retainExtensions.push_back(layer);
				}
			}

#if (NDEBUG)
			/* print supported extensions */
			std::cout
				<< "Selected device extensions: "
				<< std::endl;

			std::for_each(retainExtensions.begin(),
			              retainExtensions.end(),
			              [](auto const& e)
			              {
				              std::cout << e << std::endl;
			              }
			);

			std::cout << std::endl;
#endif

			extensions.clear();

			for (auto layer : retainExtensions)
			{
				extensions.push_back(layer);
			}
		}

		void setupSampling(const vk::SampleCountFlagBits desiredSampling =
			vk::SampleCountFlagBits::e64)
		{
			const auto properties = physicalDevice.getProperties();

			const auto supportedSampling =
				properties.limits.framebufferColorSampleCounts &
				properties.limits.framebufferDepthSampleCounts;

			if (supportedSampling & desiredSampling)
			{
				multiSampling = desiredSampling;
			}
			else if (supportedSampling & vk::SampleCountFlagBits::e64)
			{
				multiSampling = vk::SampleCountFlagBits::e64;
			}
			else if (supportedSampling & vk::SampleCountFlagBits::e32)
			{
				multiSampling = vk::SampleCountFlagBits::e32;
			}
			else if (supportedSampling & vk::SampleCountFlagBits::e16)
			{
				multiSampling = vk::SampleCountFlagBits::e16;
			}
			else if (supportedSampling & vk::SampleCountFlagBits::e8)
			{
				multiSampling = vk::SampleCountFlagBits::e8;
			}
			else if (supportedSampling & vk::SampleCountFlagBits::e4)
			{
				multiSampling = vk::SampleCountFlagBits::e4;
			}
			else if (supportedSampling & vk::SampleCountFlagBits::e2)
			{
				multiSampling = vk::SampleCountFlagBits::e2;
			}
			else
			{
				multiSampling = vk::SampleCountFlagBits::e1;
			}
		}

		void createDevice(vk::PhysicalDevice physicalDevice,
		                  vk::Instance instance,
		                  PreferredQueueFamilySettings
		                  preferredQueueFamilySetting,
		                  uint32_t graphicsQueueFamilyIndex,
		                  uint32_t transferQueueFamilyIndex)
		{
			this->physicalDevice = physicalDevice;
			this->graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;

			std::vector<vk::DeviceQueueCreateInfo> deviceQueues;

			if (preferredQueueFamilySetting ==
				PreferredQueueFamilySettings::eGraphicsTransferTogether)
			{
				const std::array<const float, 3> queuePriority = {
					0.0f, 0.0f, 0.0f
				};

				const vk::DeviceQueueCreateInfo deviceGraphicQueueCreateInfo
				{
					.queueFamilyIndex = graphicsQueueFamilyIndex,
					.queueCount = 3,
					.pQueuePriorities = queuePriority.data()
				};

				deviceQueues.push_back(deviceGraphicQueueCreateInfo);
			}
			else
			{
				const std::array<const float, 2> queuePriority = {0.0f, 0.0f};

				const vk::DeviceQueueCreateInfo deviceGraphicQueueCreateInfo
				{
					.queueFamilyIndex = graphicsQueueFamilyIndex,
					.queueCount = 2,
					.pQueuePriorities = queuePriority.data()
				};

				const auto queuePriorityT = 0.0f;

				const vk::DeviceQueueCreateInfo deviceTransferQueueCreateInfo
				{
					.queueFamilyIndex = transferQueueFamilyIndex,
					.queueCount = 1,
					.pQueuePriorities = &queuePriorityT
				};

				deviceQueues.push_back(deviceGraphicQueueCreateInfo);
				deviceQueues.push_back(deviceTransferQueueCreateInfo);
			}

			std::vector<const char*> deviceExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
			};

			filterDeviceExtensions(deviceExtensions);

#if (NDEBUG)
			/* List device validation layers */
			auto deviceLayers = physicalDevice.enumerateDeviceLayerProperties();

			std::cout
				<< "Supported devices layers: "
				<< std::endl;

			for (const auto& layer : deviceLayers)
			{
				std::cout << layer.layerName << std::endl;
			}
			std::cout << std::endl;
#endif

			auto deviceFeatures = physicalDevice.getFeatures();

			const vk::DeviceCreateInfo deviceCreateInfo{
				.queueCreateInfoCount = static_cast<uint32_t>(deviceQueues.
					size()),
				.pQueueCreateInfos = deviceQueues.data(),
				.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.
					size()),
				.ppEnabledExtensionNames = deviceExtensions.data(),
				.pEnabledFeatures = &deviceFeatures
			};

			logicalDevice = physicalDevice.createDevice(deviceCreateInfo);

			allocator = alloc::init(physicalDevice, logicalDevice, instance);

#if (NDEBUG)
			std::cout << "Logical device created!" << std::endl;
#endif

			setupSampling();

			createCommandPool();
		}

		void createCommandPool()
		{
			const vk::CommandPoolCreateInfo commandPoolCreateInfo = {
				.queueFamilyIndex = graphicsQueueFamilyIndex
			};

			commandPool = logicalDevice.
				createCommandPool(commandPoolCreateInfo);
		}

		vk::Result acquireNextImageKHR(const vk::SwapchainKHR swapchain,
		                               const vk::Semaphore
		                               imageAvailableSemaphore,
		                               uint32_t* imageIndex) const
		{
			return logicalDevice.acquireNextImageKHR(swapchain,
			                                         UINT64_MAX,
			                                         imageAvailableSemaphore,
			                                         nullptr,
			                                         imageIndex);
		}

		vk::CommandBuffer createCommandBuffer() const
		{
			const vk::CommandBufferAllocateInfo commandBufferAllocateInfo = {
				.commandPool = commandPool,
				.commandBufferCount = 1
			};

			return logicalDevice.allocateCommandBuffers(
				                     commandBufferAllocateInfo).
			                     front();
		}

		void freeCommandBuffer(vk::CommandBuffer commandBuffer) const
		{
			logicalDevice.freeCommandBuffers(commandPool, 1, &commandBuffer);
		}

		vk::CommandBuffer beginOneTimeSubmitCommands() const
		{
			const auto commandBuffer = createCommandBuffer();

			const vk::CommandBufferBeginInfo bufferBeginInfo = {
				.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
			};

			commandBuffer.begin(bufferBeginInfo);

			return commandBuffer;
		}

		void endOneTimeSubmitCommands(const vk::CommandBuffer commandBuffer,
		                              const vk::Queue queue) const
		{
			commandBuffer.end();

			vk::SubmitInfo submitInfo = {
				.commandBufferCount = 1,
				.pCommandBuffers = &commandBuffer
			};

			const auto result = queue.submit(1, &submitInfo, nullptr);
			queue.waitIdle();

			freeCommandBuffer(commandBuffer);
		}

		void copyCpuToGpuBuffer(const vk::Queue transferQueue,
		                        const alloc::Buffer src,
		                        const alloc::Buffer dst,
		                        const vk::DeviceSize size) const
		{
			const auto commandBuffer = beginOneTimeSubmitCommands();

			const vk::BufferCopy bufferCopy = {
				.size = size
			};

			commandBuffer.copyBuffer(src.buffer, dst.buffer, 1, &bufferCopy);

			endOneTimeSubmitCommands(commandBuffer, transferQueue);
		}

		void copyCpuBufferToGpuImage(const vk::Queue transferQueue,
		                             const alloc::Buffer src,
		                             const alloc::Image dst,
		                             const vk::BufferImageCopy bufferImageCopy)
		const
		{
			const auto commandBuffer = beginOneTimeSubmitCommands();

			commandBuffer.copyBufferToImage(src.buffer, dst.image,
			                                vk::ImageLayout::
			                                eTransferDstOptimal,
			                                1, &bufferImageCopy);

			endOneTimeSubmitCommands(commandBuffer, transferQueue);
		}

		void transitionImageLayout(
			const vk::Queue transferQueue,
			const vk::Image image,
			const vk::ImageLayout oldLayout,
			const vk::ImageLayout newLayout,
			const vk::ImageSubresourceRange subresourceRange) const
		{
			const auto commandBuffer = beginOneTimeSubmitCommands();

			vk::ImageMemoryBarrier imageMemoryBarrier = {
				.oldLayout = oldLayout,
				.newLayout = newLayout,
				.image = image,
				.subresourceRange = subresourceRange
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

			endOneTimeSubmitCommands(commandBuffer, transferQueue);
		}

		alloc::Buffer transferDataSetToGpuBuffer(
			const vk::Queue transferQueue,
			void* data,
			const vk::DeviceSize size,
			const vk::BufferUsageFlagBits usageFlag) const
		{
			const auto stagingVertexBuffer =
				alloc::allocateStagingTransferBuffer(allocator, data, size);
			const auto indexBuffer = alloc::createGpuBufferDst(allocator, size,
			                                                   usageFlag);
			copyCpuToGpuBuffer(transferQueue, stagingVertexBuffer, indexBuffer,
			                   size);
			alloc::deallocateBuffer(allocator, stagingVertexBuffer);

			return indexBuffer;
		}

		void waitIdle() const
		{
			logicalDevice.waitIdle();
		}

		void destroyImage(const alloc::Image image) const
		{
			alloc::deallocateImage(allocator, image);
		}

		void destroyBuffer(const alloc::Buffer buffer) const
		{
			alloc::deallocateBuffer(allocator, buffer);
		}

		void destroy() const
		{
			logicalDevice.destroyCommandPool(commandPool);
			allocator.destroy();
			logicalDevice.destroy();
		}
	};
}
