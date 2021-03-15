#pragma once
#include "Geometry.h"
#include "BaseMaterial.h"

namespace mvk
{
	class Mesh
	{
	public:
		Geometry* geometry;
		Material* material;

		Mesh(Geometry* geometry, Material* material = new BaseMaterial);

		void load(vma::Allocator allocator, vk::Device device,
		          vk::CommandPool commandPool, vk::Queue transferQueue) const;

		void release() const;

		void writeDescriptorSet(vk::DescriptorSet descriptorSet);
	};
}
