#version 450
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_separate_shader_objects : enable

const float M_PI = 3.141592653589793;

#define MANUAL_SRGB 1

layout(location = 0) in vec3 inWordPosition;
layout(location = 1) in vec3 inEyePosition;
layout(location = 2) in vec3 inFragColor;
layout(location = 3) in vec2 inUV0;
layout(location = 4) in vec3 inNormal;

layout(location = 0) out vec4 outColor;

layout(set = 2, binding = 0) uniform sampler2D baseColorMap;
layout(set = 2, binding = 1) uniform sampler2D normalMap;
layout(set = 2, binding = 2) uniform sampler2D metallicRoughnessMap;
 
 vec3 Uncharted2Tonemap(vec3 color)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((color*(A*color+C*B)+D*E)/(color*(A*color+B)+D*F))-E/F;
}

vec4 tonemap(vec4 color)
{
	vec3 outcol = Uncharted2Tonemap(color.rgb * 2.8);
	outcol = outcol * (1.0f / Uncharted2Tonemap(vec3(11.2f)));	
	return vec4(pow(outcol, vec3(1.0f / 2.2)), color.a);
}

vec4 sRgbToLinear(vec4 srgbIn)
{
	vec3 bLess = step(vec3(0.04045),srgbIn.xyz);
	vec3 linOut = mix( srgbIn.xyz/vec3(12.92), pow((srgbIn.xyz+vec3(0.055))/vec3(1.055),vec3(2.4)), bLess );
	return vec4(linOut,srgbIn.w);;
}

vec3 getNormal()
{
	// http://www.thetenthplanet.de/archives/1180
	vec3 tangentNormal = texture(normalMap, inUV0).xyz * 2.0 - 1.0;
	tangentNormal.y = -tangentNormal.y;

	vec3 q1 = dFdx(inWordPosition);
	vec3 q2 = dFdy(inWordPosition);
	vec2 st1 = dFdx(inUV0);
	vec2 st2 = dFdy(inUV0);

	vec3 N = normalize(inNormal);
	vec3 T = normalize(q1 * st2.t - q2 * st1.t);
	vec3 B = -normalize(cross(N, T));
	mat3 TBN = mat3(T, B, N);

	return normalize(TBN * tangentNormal);
}

void main() {

	outColor = vec4(0);

	vec4 baseColor = sRgbToLinear(texture(baseColorMap, inUV0));
	vec4 mR = texture(metallicRoughnessMap, inUV0);

	float occlusion = mR.x;
	float roughness = mR.y;
	float metallic = mR.z;
   
	vec3 lightDir = normalize(vec3(0., -.5, -.5));

	vec3 n = getNormal();
	vec3 v = normalize(inEyePosition - inWordPosition); // surface to eye
	vec3 l = -lightDir; // surface to light
	vec3 h = normalize(l+v); // half vector

	float dotL = clamp(dot(n, l), 0.001, 1.0);
	float dotV = clamp(abs(dot(n, v)), 0.001, 1.0);
	float dotH = clamp(dot(n, h), 0., 1.);

	// F0 - lerp between dielectric and metallic
	// for metallic materials F0 is coded in the baseColor map
	vec3 f0 = mix(vec3(0.04), baseColor.rgb, metallic);
	float a = roughness * roughness;

	// Fresnel
	vec3 F = f0 + (1. - f0) * pow((1. - abs(dotL)),5.);

	// Geometric occlusion
	float attenuationL = 2.0 * dotL / (dotL + sqrt(a + (1.0 - a) * (dotL * dotL)));
	float attenuationV = 2.0 * dotV / (dotV + sqrt(a + (1.0 - a) * (dotV * dotV)));
	float G = attenuationL * attenuationV;
	
	// Micorfacet Distribution
	float d = (dotH * a - dotH) * dotH + 1.0;
	float D = a / (M_PI * d * d);

	// Get Diffuse for dielectric parts and set to 0 for metallic
	vec3 diffuse = mix(baseColor.rgb, vec3(0), metallic);
	
	diffuse = (1.0 - F) * (1.0 / M_PI) * diffuse;
	
	vec3 specular = F * G * D / (4.0 * dotL * dotV);

	// Save shiny
	//specular = pow(dotV, 128.0) * baseColor.rgb + baseColor.rgb * 0.2;
	//outColor.rgb = dotL * diffuse  + specular;
	outColor.rgb = dotL * (diffuse  + specular);
	outColor = mix(outColor * 0.1, outColor, occlusion);
	outColor.a = baseColor.a;
	//outColor = baseColor;
	//outColor = tonemap(outColor);
}