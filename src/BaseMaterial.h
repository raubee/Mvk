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
		inline static BaseMaterialDescription defaultDescription{
			.albedo = nullptr
		};

	public:
		BaseMaterial(vk::Device device,
		             BaseMaterialDescription description = defaultDescription);

		Texture2D* albedo;

		void init(vk::Device device, vma::Allocator allocator) override;
		void createDescriptorPool(vk::Device device) override;
		void updateDescriptorSets(vk::Device device) override;
		void createDescriptorSetLayout(vk::Device device) override;
		void createDescriptorSets(vk::Device device) override;
		void release(vk::Device device) override;
	};
}
