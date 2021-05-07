#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 eye;
} ubo;

layout(set = 1, binding = 0) uniform localMatrixBufferObject {
    mat4 matrix;
} nodeUbo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inUV0;

layout(location = 0) out vec3 worldPosition;
layout(location = 1) out vec3 eyePosition;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out vec3 normal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	vec4 localPos = nodeUbo.matrix * vec4(inPosition, 1.0);
	worldPosition = localPos.xyz / localPos.w;
    fragColor = inColor;
    fragTexCoord = inUV0;
	normal = normalize(transpose(inverse(mat3(ubo.model * nodeUbo.matrix))) * inNormal);
	eyePosition = ubo.eye.xyz;
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(worldPosition, 1.0);
}