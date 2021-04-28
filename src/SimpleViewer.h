#pragma once

#include "AppBase.h"
#include "NormalMaterial.h"

class SimpleViewer : public mvk::AppBase
{
public:
	SimpleViewer(const mvk::Context context,
	             const vk::SurfaceKHR surface): AppBase(context, surface)
	{
		const auto vertices = std::vector<mvk::Vertex>({
			{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
			{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
			{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
		});

		const auto indices = std::vector<uint16_t>({
			0, 1, 2, 2, 3, 0,
			4, 5, 6, 6, 7, 4
		});

		const auto vertexBuffer = createVertexBufferObject(vertices);
		const auto indexBuffer = createIndexBufferObject(indices);
		auto planeGeo = mvk::Geometry(vertexBuffer, vertices.size(),
		                         indexBuffer, indices.size());

		auto material = mvk::NormalMaterial(device);
		material.init(device, allocator);

		std::array<vk::DescriptorSetLayout, 1> descriptorSetLayouts = {
		scene.getDescriptorSetLayout()
		};

		mvk::GraphicPipeline graphicPipeline(device,
			swapchain.getSwapchainExtent(),
			renderPass.getRenderPass(),
			material.getPipelineShaderStageCreateInfo(),
			descriptorSetLayouts.data(),
			static_cast<int32_t>(descriptorSetLayouts.
				size()));

		auto plane = mvk::Mesh(&planeGeo, &material, &graphicPipeline);

		scene.addObject(&plane);

		setupCommandBuffers();
	}
};
