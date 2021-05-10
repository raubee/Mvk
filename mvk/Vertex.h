#pragma once

#include "Vulkan.h"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

namespace mvk
{
	struct Vertex
	{
		glm::vec3 position; // location = 0
		glm::vec3 color; // 1
		glm::vec3 normal; // 2 
		glm::vec2 texCoord; // 3
		glm::vec2 texCoord1; // 4

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			vk::VertexInputBindingDescription vertexInputBindingDescription{
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = vk::VertexInputRate::eVertex
			};

			return vertexInputBindingDescription;
		}

		static std::vector<vk::VertexInputAttributeDescription>
		getAttributeDescriptions()
		{
			std::vector<vk::VertexInputAttributeDescription>
				vertexInputAttributeDescriptions = {
					// Position
					{
						.location = 0,
						.binding = 0,
						.format = vk::Format::eR32G32B32Sfloat,
						.offset = offsetof(Vertex, position)
					},
					// Vertex Color
					{
						.location = 1,
						.binding = 0,
						.format = vk::Format::eR32G32B32Sfloat,
						.offset = offsetof(Vertex, color)
					},
					// Normal
					{
						.location = 2,
						.binding = 0,
						.format = vk::Format::eR32G32B32Sfloat,
						.offset = offsetof(Vertex, normal)
					},
					// UV0
					{
						.location = 3,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, texCoord)
					},
					// UV0
					{
						.location = 4,
						.binding = 0,
						.format = vk::Format::eR32G32Sfloat,
						.offset = offsetof(Vertex, texCoord1)
					}
				};

			return vertexInputAttributeDescriptions;
		}
	};
}
