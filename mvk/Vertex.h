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

		static vk::VertexInputBindingDescription getBindingDescription()
		{
			vk::VertexInputBindingDescription vertexInputBindingDescription{
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = vk::VertexInputRate::eVertex
			};

			return vertexInputBindingDescription;
		}

		static std::array<vk::VertexInputAttributeDescription, 4>
		getAttributeDescriptions()
		{
			std::array<vk::VertexInputAttributeDescription, 4>
				vertexInputAttributeDescriptions = {};

			vertexInputAttributeDescriptions[0] = vk::
				VertexInputAttributeDescription{
					.location = 0,
					.binding = 0,
					.format = vk::Format::eR32G32B32Sfloat,
					.offset = offsetof(Vertex, position)
				};

			vertexInputAttributeDescriptions[1] = vk::
				VertexInputAttributeDescription{
					.location = 1,
					.binding = 0,
					.format = vk::Format::eR32G32B32Sfloat,
					.offset = offsetof(Vertex, color)
				};

			vertexInputAttributeDescriptions[2] = vk::
				VertexInputAttributeDescription{
					.location = 2,
					.binding = 0,
					.format = vk::Format::eR32G32B32Sfloat,
					.offset = offsetof(Vertex, normal)
			};

			vertexInputAttributeDescriptions[3] = vk::
				VertexInputAttributeDescription{
					.location = 3,
					.binding = 0,
					.format = vk::Format::eR32G32Sfloat,
					.offset = offsetof(Vertex, texCoord)
			};

			return vertexInputAttributeDescriptions;
		}
	};
}
