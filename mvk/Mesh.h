#pragma once

#include "Vertex.h"
#include "Material.h"
#include "GraphicPipeline.h"

namespace mvk
{
	class Mesh
	{
	public:

		alloc::Buffer vertexBuffer;
		alloc::Buffer indexBuffer;

		uint32_t verticesCount;
		uint32_t indicesCount;

		Material* material;
		GraphicPipeline* graphicPipeline;

		void loadFromFile(vma::Allocator allocator, vk::Device device,
		                  vk::CommandPool commandPool,
		                  vk::Queue transferQueue,
		                  const char* filePath);

		void setMaterial(Material* material) { this->material = material; }

		void setGraphicPipeline(GraphicPipeline* graphicPipeline)
		{
			this->graphicPipeline = graphicPipeline;
		}

		void release(vma::Allocator allocator) const;
	};
}
