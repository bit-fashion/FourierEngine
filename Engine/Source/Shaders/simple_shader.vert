#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 m;
    mat4 v;
    mat4 p;
} ubo;

layout(location = 0) out vec3 outColor;
layout(location = 1) out float outTime;

void main() {
    gl_Position = ubo.p * ubo.v * ubo.m * vec4(inPosition, 1.0f);
    outColor = inColor;
}