#include "Mesh.h"

using namespace mvk;

Mesh::Mesh(Geometry* geometry, Material* material,
           GraphicPipeline* graphicPipeline)
{
	this->geometry = geometry;
	this->material = material;
	this->graphicPipeline = graphicPipeline;
}

void Mesh::release(const vk::Device device,
                   const vma::Allocator allocator) const
{
	this->geometry->release(allocator);
	this->material->release(device);
}
