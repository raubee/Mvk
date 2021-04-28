#pragma once

#include "AppBase.h"

using namespace mvk;

class SimpleViewer : public mvk::AppBase
{
public:
	SimpleViewer(const mvk::Context context,
	             const vk::SurfaceKHR surface): AppBase(context, surface)
	{
		const auto vertices = std::vector<Vertex>({
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
		auto planeGeo = Geometry(vertexBuffer, vertices.size(),
		                         indexBuffer, indices.size());

		const auto albedo = Texture2D("assets/textures/lena.jpg");
		const BaseMaterialDescription baseMaterialDescription = {
			.albedo = albedo
		};

		auto baseMaterial = BaseMaterial(nullptr);
		const auto plane = new Mesh(&planeGeo, &baseMaterial);

		scene.addObject(plane);
	}
};
