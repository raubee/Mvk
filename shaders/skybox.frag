#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform samplerCube textureMap;

layout(location = 0) in vec3 inUV0;

layout(location = 0) out vec4 outColor;

vec4 sRgbToLinear(vec4 srgbIn)
{
	vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	return vec4(linOut,srgbIn.w);;
}

void main() {
	outColor = sRgbToLinear(texture(textureMap, inUV0));
	//outColor = vec4(inUV0, 0.,0.);
}