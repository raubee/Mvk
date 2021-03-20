#include "Geometry.h"

using namespace mvk;

Geometry::Geometry(std::vector<Vertex>* vertices,
                   std::vector<uint16_t>* indices)
{
	this->vertices = vertices;
	this->indices = indices;
}

void Geometry::load(const vma::Allocator allocator,
                    const vk::Device device,
                    const vk::CommandPool commandPool,
                    const vk::Queue transferQueue)
{
	this->allocator = allocator;
	this->device = device;
	this->commandPool = commandPool;
	this->transferQueue = transferQueue;

	createVertexBufferObject();
	createIndexBuffersObject();
}

void Geometry::release()
{
	for (const auto vertexBuffer : vertexBuffers)
	{
		deallocateBuffer(allocator, vertexBuffer);
	}

	deallocateBuffer(allocator, indexBuffer);
}

void Geometry::createVertexBufferObject()
{
	const auto size =
		static_cast<vk::DeviceSize>(sizeof vertices->at(0) * vertices->
			size());
	const auto stagingVertexBuffer =
		allocateStagingTransferBuffer(allocator, vertices->data(), size);
	const auto vertexBuffer = createGpuVertexBuffer(size);
	copyCpuToGpuBuffer(allocator, device, commandPool, transferQueue,
	                   stagingVertexBuffer, vertexBuffer,
	                   size);
	deallocateBuffer(allocator, stagingVertexBuffer);

	this->vertexBuffers.push_back(vertexBuffer);
}

void Geometry::createIndexBuffersObject()
{
	const auto size =
		static_cast<vk::DeviceSize>(sizeof indices->at(0) * indices->
			size());
	const auto stagingVertexBuffer =
		allocateStagingTransferBuffer(allocator, indices->data(), size);
	const auto indexBuffer = createGpuIndexBuffer(size);
	copyCpuToGpuBuffer(allocator, device, commandPool, transferQueue,
	                   stagingVertexBuffer, indexBuffer, size);
	deallocateBuffer(allocator, stagingVertexBuffer);

	this->indexBuffer = indexBuffer;
}

AllocatedBuffer Geometry::createGpuVertexBuffer(const vk::DeviceSize size) const
{
	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eTransferDst |
		vk::BufferUsageFlagBits::eVertexBuffer,
		.sharingMode = vk::SharingMode::eExclusive
	};

	const auto buffer =
		allocateGpuOnlyBuffer(allocator, bufferCreateInfo);

	return buffer;
}

AllocatedBuffer Geometry::createGpuIndexBuffer(const vk::DeviceSize size) const
{
	const vk::BufferCreateInfo bufferCreateInfo = {
		.size = static_cast<vk::DeviceSize>(size),
		.usage = vk::BufferUsageFlagBits::eTransferDst |
		vk::BufferUsageFlagBits::eIndexBuffer,
		.sharingMode = vk::SharingMode::eExclusive
	};

	const auto buffer =
		allocateGpuOnlyBuffer(allocator, bufferCreateInfo);

	return buffer;
}
