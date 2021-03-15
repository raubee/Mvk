#include "Mesh.h"

using namespace mvk;

Mesh::Mesh(Geometry* geometry, Material* material)
{
	this->geometry = geometry;
	this->material = material;
}

void Mesh::load(const vma::Allocator allocator,
                const vk::Device device,
                const vk::CommandPool commandPool,
                const vk::Queue transferQueue) const
{
	this->geometry->load(allocator, device, commandPool, transferQueue);
	this->material->load(allocator, device, commandPool, transferQueue);
}

void Mesh::release() const
{
	this->geometry->release();
	this->material->release();
}

void Mesh::writeDescriptorSet(const vk::DescriptorSet descriptorSet)
{
	this->material->writeDescriptorSet(descriptorSet);
}
