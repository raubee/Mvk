#include "NormalMaterial.h"

void mvk::NormalMaterial::load(const Device device)
{
	Material::load(
		new Shader(device, "shaders/base.vert.spv",
		           vk::ShaderStageFlagBits::eVertex),
		new Shader(device, "shaders/norm.frag.spv",
		           vk::ShaderStageFlagBits::eFragment));
}
