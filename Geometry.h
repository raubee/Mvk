#pragma once

#include "VulkanVma.h"
#include "Vertex.h"

namespace mvk
{
	class Geometry
	{
		vma::Allocator allocator;
		vk::Device device;
		vk::CommandPool commandPool;
		vk::Queue transferQueue;

		std::vector<Vertex>* vertices;
		std::vector<uint16_t>* indices;

		std::vector<AllocatedBuffer> vertexBuffers;
		AllocatedBuffer indexBuffer;

		void createVertexBufferObject();
		void createIndexBuffersObject();
		AllocatedBuffer createGpuVertexBuffer(vk::DeviceSize size) const;
		AllocatedBuffer createGpuIndexBuffer(vk::DeviceSize size) const;

	public:
		Geometry(std::vector<Vertex>* vertices, std::vector<uint16_t>* indices);
		std::vector<Vertex>* getVertices() const { return vertices; }
		std::vector<uint16_t>* getIndices() const { return indices; }

		std::vector<AllocatedBuffer> getVertexBuffers() const
		{
			return vertexBuffers;
		}

		AllocatedBuffer getIndexBuffer() const { return indexBuffer; }

		void load(vma::Allocator allocator, vk::Device device,
		          vk::CommandPool commandPool, vk::Queue transferQueue);
		void release();
	};
}
