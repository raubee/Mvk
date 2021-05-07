#pragma once

#include "Material.h"
#include "Texture2D.h"

namespace mvk
{
	enum AlphaMode
	{
		NO_ALPHA,
		ALPHA_BLEND,
		ALPHA_CUTOFF
	};

	struct BaseMaterialDescription
	{
		AlphaMode alphaMode = NO_ALPHA;

		Texture2D* baseColor = Texture2D::empty;
		Texture2D* normal = Texture2D::empty;
		Texture2D* metallicRoughness = Texture2D::empty;

		Texture2D* occlusion = Texture2D::empty;
		Texture2D* emissive = Texture2D::empty;
	};

	class BaseMaterial : public Material
	{
		inline static vk::DescriptorSetLayout descriptorSetLayout;

		void createDescriptorPool();
		void createDescriptorSets();
		void updateDescriptorSets();

	public:

		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;

		AlphaMode alphaMode;

		Texture2D* baseColor;
		Texture2D* normal;
		Texture2D* metallicRoughness;

		void load(Device* device, BaseMaterialDescription description);

		void release() override;

		static vk::DescriptorSetLayout getDescriptorSetLayout(Device* device)
		{
			if (!descriptorSetLayout)
				createDescriptorSetLayout(device);

			return descriptorSetLayout;
		}

		static void createDescriptorSetLayout(Device* device);

		vk::DescriptorSet getDescriptorSet() const
		{
			return descriptorSets[0];
		}
	};
}
