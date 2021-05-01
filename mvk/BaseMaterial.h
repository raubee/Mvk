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

		void load(Device device,
		          BaseMaterialDescription description = defaultDescription);

		void createDescriptorPool(Device device);
		void updateDescriptorSets(Device device);
		void createDescriptorSetLayout(Device device);
		void createDescriptorSets(Device device);
	};
}
