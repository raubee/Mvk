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

		void createDescriptorPool(vk::Device device);
		void updateDescriptorSets(vk::Device device);
		void createDescriptorSetLayout(vk::Device device);
		void createDescriptorSets(vk::Device device);
		void release(vk::Device device) override;
	};
}
