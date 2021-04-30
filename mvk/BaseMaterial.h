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
		
		Texture2D* albedo;

		void load(vk::Device device,
		          BaseMaterialDescription description = defaultDescription);

		void createDescriptorPool(vk::Device device);
		void updateDescriptorSets(vk::Device device);
		void createDescriptorSetLayout(vk::Device device);
		void createDescriptorSets(vk::Device device);
	};
}
