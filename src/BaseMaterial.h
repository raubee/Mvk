#pragma once

#include "Material.h"
#include "Texture2D.h"

namespace mvk
{
	struct BaseMaterialDescription
	{
		std::string albedo;
	};

	class BaseMaterial : public Material
	{
		BaseMaterialDescription description;

	public:
		inline static BaseMaterialDescription defaultDescription{};

		Texture2D albedo;

		void setDescription(const BaseMaterialDescription description)
		{
			this->description = description;
		}

		void load(vma::Allocator allocator, vk::Device device,
		          vk::CommandPool commandPool,
		          vk::Queue transferQueue) override;

		void writeDescriptorSet(vk::DescriptorSet descriptorSet) override;
		
		void release() override;
	};
}
