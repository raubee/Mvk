#pragma once

#include "Shader.h"

#define VMA_HPP_NO_STRUCT_CONSTRUCTORS
#include "../3rdParty/vk_mem_alloc.hpp"

namespace mvk
{
	class Material
	{
	protected:

		Shader* vertShader;
		Shader* fragShader;
		Shader* geoShader;
		Shader* tesShader;
	
	public:
		virtual void load(Shader* vertShader,
		                  Shader* fragShader,
		                  Shader* geoShader = nullptr,
		                  Shader* tesShader = nullptr);

		virtual void release(Device device);

		std::vector<vk::PipelineShaderStageCreateInfo>
		getPipelineShaderStageCreateInfo() const;
	};
}
