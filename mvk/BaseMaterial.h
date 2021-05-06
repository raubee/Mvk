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

		inline static vk::DescriptorSetLayout descriptorSetLayout;

		void createDescriptorPool();
		void createDescriptorSets();
		void updateDescriptorSets();

	public:

		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;

		Texture2D* albedo;

		void load(Device* device,
		          BaseMaterialDescription description = defaultDescription);

		void release() override;

		static vk::DescriptorSetLayout getDescriptorSetLayout(Device* device)
		{
			if (!descriptorSetLayout)
				createDescriptorSetLayout(device);

			return descriptorSetLayout;
		}

		static void createDescriptorSetLayout(Device* device)
		{
			const vk::DescriptorSetLayoutBinding albedoLayoutBinding = {
				.binding = 0,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eFragment
			};

			std::vector<vk::DescriptorSetLayoutBinding> layoutBindings
				= {albedoLayoutBinding};

			const auto bindingCount =
				static_cast<uint32_t>(layoutBindings.size());

			const vk::DescriptorSetLayoutCreateInfo
				descriptorSetLayoutCreateInfo = {
					.bindingCount = bindingCount,
					.pBindings = layoutBindings.data()
				};

			descriptorSetLayout = device->logicalDevice
			                            .createDescriptorSetLayout(
				                            descriptorSetLayoutCreateInfo);
		}

		vk::DescriptorSet getDescriptorSet() const
		{
			return descriptorSets[0];
		}
	};
}
