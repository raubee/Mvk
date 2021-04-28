#pragma once
#include "Geometry.h"
#include "BaseMaterial.h"

namespace mvk
{
	class Mesh
	{
		std::vector<vk::DescriptorSet> descriptorSets;

	public:
		Geometry* geometry;
		Material* material;

		Mesh(Geometry* geometry, Material* material);

		void release(vk::Device device, vma::Allocator allocator) const;

		vk::DescriptorSet getDescriptorSet(const int i)
		{
			return descriptorSets[i];
		}
	};
}
