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

	public:

		vk::DescriptorPool descriptorPool;
		std::vector<vk::DescriptorSet> descriptorSets;

		Texture2D* albedo;

		void load(Device device,
		          BaseMaterialDescription description = defaultDescription);

		void release(Device device) override;

		void createDescriptorPool(Device device);
		void createDescriptorSets(Device device);
		void updateDescriptorSets(Device device);

		static vk::DescriptorSetLayout getDescriptorSetLayout(
			const Device device)
		{
			if (!descriptorSetLayout)
				createDescriptorSetLayout(device);

			return descriptorSetLayout;
		}

		static void createDescriptorSetLayout(const Device device)
		{
			const vk::DescriptorSetLayoutBinding albedoLayoutBinding = {
				.binding = 0,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.descriptorCount = 1,
				.stageFlags = vk::ShaderStageFlagBits::eFragment
			};

			std::array<vk::DescriptorSetLayoutBinding, 1> layoutBindings
				= {albedoLayoutBinding};

			const vk::DescriptorSetLayoutCreateInfo
				descriptorSetLayoutCreateInfo = {
					.bindingCount = static_cast<uint32_t>(layoutBindings.size()
					),
					.pBindings = layoutBindings.data()
				};

			descriptorSetLayout = vk::Device(device)
				.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
		}

		vk::DescriptorSet getDescriptorSet(const int index) const
		{
			if (descriptorSets.size() <= index) return nullptr;

			return descriptorSets[index];
		}
	};
}
