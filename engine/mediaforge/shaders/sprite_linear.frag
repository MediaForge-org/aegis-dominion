#version 450

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv;
layout(set = 2, binding = 0) uniform sampler2D spriteTexture;
layout(location = 0) out vec4 outColor;

vec3 srgbToLinear(vec3 value) {
    return mix(value / 12.92, pow((value + 0.055) / 1.055, vec3(2.4)), step(vec3(0.04045), value));
}

void main() {
    vec4 sampleColor = texture(spriteTexture, uv);
    outColor = vec4(srgbToLinear(sampleColor.rgb) * color.rgb, sampleColor.a * color.a);
}
