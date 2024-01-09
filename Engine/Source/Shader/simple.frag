#version 450

layout(location = 0) in vec3 inColor;
layout(location = 1) in float inTime;
layout(location = 2) in vec2 inTexCoord;

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, inTexCoord);
}