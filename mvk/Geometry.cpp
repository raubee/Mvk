#include "Geometry.h"

using namespace mvk;

Geometry::Geometry(const alloc::Buffer vertexBuffer,
                   const uint32_t verticesCount,
                   const alloc::Buffer indexBuffer,
                   const uint32_t indicesCount)
{
	this->vertexBuffer = vertexBuffer;
	this->indexBuffer = indexBuffer;

	this->verticesCount = verticesCount;
	this->indicesCount = indicesCount;
}

void Geometry::release(const vma::Allocator allocator) const
{
	deallocateBuffer(allocator, vertexBuffer);
	deallocateBuffer(allocator, indexBuffer);
}
