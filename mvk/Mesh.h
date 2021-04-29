#pragma once

#include "Geometry.h"
#include "Material.h"
#include "GraphicPipeline.h"

namespace mvk
{
	class Mesh
	{

	public:

		Geometry* geometry;
		Material* material;
		GraphicPipeline* graphicPipeline;

		Mesh(Geometry* geometry, Material* material,
		     GraphicPipeline* graphicPipeline);

		void release(vk::Device device, vma::Allocator allocator) const;
	};
}
