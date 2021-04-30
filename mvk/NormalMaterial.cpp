#include "NormalMaterial.h"

void mvk::NormalMaterial::load(const vk::Device device)
{
	Material::load(
		device,
		new Shader(device, "shaders/vert.spv",
		           vk::ShaderStageFlagBits::eVertex),
		new Shader(device, "shaders/frag-norm.spv",
		           vk::ShaderStageFlagBits::eFragment));
}
