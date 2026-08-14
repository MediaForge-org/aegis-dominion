#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec4 color;
layout(location = 1) out vec2 uv;

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0);
    color = inColor;
    uv = inUv;
}
