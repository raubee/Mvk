#pragma once

#include "Material.h"
#include "Texture2D.h"

namespace mvk
{
	struct BaseMaterialDescription
	{
		Texture2D* albedo;
	};

	class BaseMaterial : public Material
	{
		BaseMaterialDescription description;

		inline static BaseMaterialDescription defaultDescription{
			.albedo = nullptr
		};

	public:
		BaseMaterial(vk::Device device,
		             vma::Allocator allocator,
		             BaseMaterialDescription description = defaultDescription);

		Texture2D* albedo;

		void init(vk::Device device, vma::Allocator allocator,
		          uint32_t size) override;
		void createDescriptorPool(vk::Device device, uint32_t size);
		void writeDescriptorSet(vk::Device device,
		                        vk::DescriptorSet descriptorSet);
		vk::DescriptorSetLayout getDescriptorSetLayout(vk::Device device)
		override;
		void release(vk::Device device) override;
	};
}
