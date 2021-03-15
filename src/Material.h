#pragma once
#include "Shader.h"

namespace mvk
{
	class Material
	{
		Shader* vertShader;
		Shader* fragShader;
		Shader* geoShader;
		Shader* tesShader;
		
	protected:
		vma::Allocator allocator;
		vk::Device device;
		vk::CommandPool commandPool;
		vk::Queue transferQueue;
		
	public:
		Material(const std::string& vertShader,
		         const std::string& fragShader,
		         const std::string& geoShader = "",
		         const std::string& tesShader = "");
		Material(Shader vertShader, Shader fragShader,
		         Shader geoShader, Shader tesShader);

		void loadShaders(vk::Device device) const;
		
		virtual void load(vma::Allocator allocator,
		                  vk::Device device,
		                  vk::CommandPool commandPool,
		                  vk::Queue transferQueue);
		
		virtual void release();

		virtual void writeDescriptorSet(vk::DescriptorSet descriptorSet){};

		std::vector<vk::PipelineShaderStageCreateInfo>
		getPipelineShaderStageCreateInfo() const;
	};
}
