#include "NormalMaterial.h"

mvk::NormalMaterial::NormalMaterial(const vk::Device device): Material(
	new Shader(device, "shaders/vert.spv",
	           vk::ShaderStageFlagBits::eVertex),
	new Shader(device, "shaders/frag-norm.spv",
	           vk::ShaderStageFlagBits::eFragment))
{
}
