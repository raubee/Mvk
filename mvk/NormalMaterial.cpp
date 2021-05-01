#include "NormalMaterial.h"

void mvk::NormalMaterial::load(const Device device)
{
	Material::load(
		new Shader(device, "shaders/vert.spv",
		           vk::ShaderStageFlagBits::eVertex),
		new Shader(device, "shaders/frag-norm.spv",
		           vk::ShaderStageFlagBits::eFragment));
}
