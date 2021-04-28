#pragma once

#pragma once

#include "Material.h"
#include "Texture2D.h"

namespace mvk
{

	class NormalMaterial : public Material
	{
	public:
		NormalMaterial(vk::Device device);
	};
}
