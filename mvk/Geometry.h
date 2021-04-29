#pragma once

#include "VulkanVma.h"

namespace mvk
{
	class Geometry
	{
		uint32_t verticesCount;
		uint32_t indicesCount;

		alloc::Buffer vertexBuffer;
		alloc::Buffer indexBuffer;

	public:
		Geometry(alloc::Buffer vertexBuffer,
		         uint32_t verticesCount,
		         alloc::Buffer indexBuffer,
		         uint32_t indicesCount);

		alloc::Buffer getVertexBuffer() const { return vertexBuffer; }
		alloc::Buffer getIndexBuffer() const { return indexBuffer; }

		uint32_t getVerticesCount() const { return verticesCount; }
		uint32_t getIndexCount() const { return indicesCount; }
		void release(vma::Allocator allocator) const;
	};
}
