#pragma once

#include "VulkanVma.h"
#include "Vertex.h"

namespace mvk
{
	class Geometry
	{
		size_t verticesCount;
		size_t indicesCount;

		alloc::Buffer vertexBuffer;
		alloc::Buffer indexBuffer;

	public:
		Geometry(alloc::Buffer vertexBuffer,
		         size_t verticesCount,
		         alloc::Buffer indexBuffer,
		         size_t indicesCount);

		alloc::Buffer getVertexBuffer() const { return vertexBuffer; }
		alloc::Buffer getIndexBuffer() const { return indexBuffer; }

		size_t getVerticesCount() const { return verticesCount; }
		size_t getIndexCount() const { return indicesCount; }
		void release(vma::Allocator allocator) const;
	};
}
