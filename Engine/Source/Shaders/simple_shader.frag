#version 450

layout(location = 0) in vec3 inColor;

layout(location = 0) out vec4 OUPUT_FINAL_COLOR;

void main() {
    OUPUT_FINAL_COLOR = vec4(inColor, 1.0);
}
