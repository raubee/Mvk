#pragma once

#include "Vertex.h"
#include "Material.h"
#include "GraphicPipeline.h"

namespace mvk
{
	class Model
	{
	public:

		alloc::Buffer vertexBuffer;
		alloc::Buffer indexBuffer;

		uint32_t verticesCount;
		uint32_t indicesCount;

		Material* material;
		GraphicPipeline* graphicPipeline;

		void loadFromFile(Device device, vk::Queue transferQueue,
		                  const char* filePath);

		void setMaterial(Material* material) { this->material = material; }

		void setGraphicPipeline(GraphicPipeline* graphicPipeline)
		{
			this->graphicPipeline = graphicPipeline;
		}

		void release(Device device) const;
	};
}
