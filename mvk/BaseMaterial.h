#pragma once

#include "Material.h"
#include "Texture2D.h"

#include <glm/glm.hpp>

namespace mvk
{
	enum AlphaMode
	{
		NO_ALPHA,
		ALPHA_BLEND,
		ALPHA_CUTOFF
	};

	class BaseMaterial : public Material
	{
		inline static vk::DescriptorSetLayout descriptorSetLayout;

		void createDescriptorPool();
		void createDescriptorSets();
		void updateDescriptorSets();

	public:

		struct PushConstants
		{
			glm::vec4 baseColorFactor = glm::vec4(1);
			float metallicFactor = 0.0f;
			float roughnessFactor = 1.0f;
			int baseTextureSet = -1;
			int normalTextureSet = -1;
			int metallicRoughnessTextureSet = -1;
		};

		struct BaseMaterialDescription
		{
			PushConstants constants;

			AlphaMode alphaMode = NO_ALPHA;

			Texture2D* baseColor = Texture2D::empty;
			Texture2D* normal = Texture2D::empty;
			Texture2D* metallicRoughness = Texture2D::empty;
			Texture2D* occlusion = Texture2D::empty;
			Texture2D* emissive = Texture2D::empty;
		};

		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;

		AlphaMode alphaMode;
		PushConstants constants;

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

		static vk::PushConstantRange getPushConstantRange()
		{
			vk::PushConstantRange pushConstantRange =
			{
				.stageFlags = vk::ShaderStageFlagBits::eFragment,
				.offset = 0,
				.size = sizeof(PushConstants)
			};

			return pushConstantRange;
		}
	};
}
