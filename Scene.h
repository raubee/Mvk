#pragma once

#include "Mesh.h"

namespace mvk
{
	class Scene
	{
		std::vector<Mesh*> meshes;

	public:
		Scene() : meshes(0)
		{
		}

		void addObject(Mesh* mesh) { meshes.push_back(mesh); }
		std::vector<Mesh*> getObjects() const { return meshes; }
	};
}
