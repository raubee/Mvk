#pragma once

#include "Shader.h"

#define VMA_HPP_NO_STRUCT_CONSTRUCTORS
#include "../3rdParty/vk_mem_alloc.hpp"

namespace mvk
{
	class Material
	{
	protected:

		Device* ptrDevice;

		Shader* vertShader;
		Shader* fragShader;
		Shader* geoShader;
		Shader* tesShader;

	public:
		virtual void load(Device* device,
		                  Shader* vertShader,
		                  Shader* fragShader,
		                  Shader* geoShader = nullptr,
		                  Shader* tesShader = nullptr);

		virtual void release();

		std::vector<vk::PipelineShaderStageCreateInfo>
		getPipelineShaderStageCreateInfo() const;
	};
}
