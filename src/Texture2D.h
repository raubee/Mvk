#pragma once

#include "VulkanVma.h"

namespace mvk
{
	class Texture2D
	{
		vk::Device device;
		vma::Allocator allocator;
		vk::CommandPool commandPool;
		vk::Queue queue;
		
		vk::Format format;
		AllocatedImage image;
		vk::ImageView imageView;
		vk::Sampler sampler;
		
		void loadImage(const char* filename);
		void createImageView();
		void createSampler();
	public:
		void load(vma::Allocator allocator, vk::Device device,
		          vk::CommandPool commandPool,
		          vk::Queue transferQueue, const char* filename);
		void release() const;

		vk::Sampler getSampler() const { return sampler; }
		vk::ImageView getImageView() const { return imageView; }
	};
}
