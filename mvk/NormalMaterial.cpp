#include "NormalMaterial.h"

void mvk::NormalMaterial::load(Device* device)
{
	Material::load(device,
		new Shader(device, "shaders/base.vert.spv",
		           vk::ShaderStageFlagBits::eVertex),
		new Shader(device, "shaders/norm.frag.spv",
		           vk::ShaderStageFlagBits::eFragment));
}
