#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
	vec3 eye;
} ubo;

// Vertex
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV0;

layout(location = 0) out vec3 outUVW;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
	outUVW = inPosition;
    gl_Position = ubo.proj * ubo.view * vec4(inPosition, 1.0);
}