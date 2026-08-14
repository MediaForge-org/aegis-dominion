#version 450

layout(location = 0) in vec4 color;
layout(location = 1) in vec2 uv;
layout(set = 2, binding = 0) uniform sampler2D worldTexture;
layout(set = 2, binding = 1) uniform sampler2D emissiveTexture;
layout(set = 2, binding = 2) uniform sampler2D overlayTexture;
layout(std140, set = 3, binding = 0) uniform CompositeSettings {
    vec4 gradeExposure;
    vec4 bloomVignetteSaturation;
    vec4 inverseTextureSize;
} settings;
layout(location = 0) out vec4 outColor;

vec3 linearToSrgb(vec3 value) {
    value = max(value, vec3(0.0));
    return mix(value * 12.92, 1.055 * pow(value, vec3(1.0 / 2.4)) - 0.055,
               step(vec3(0.0031308), value));
}

vec3 toneMap(vec3 value) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((value * (a * value + b)) / (value * (c * value + d) + e), 0.0, 1.0);
}

void main() {
    vec3 world = texture(worldTexture, uv).rgb;
    vec2 texel = settings.inverseTextureSize.zw * settings.bloomVignetteSaturation.y;
    vec3 glow = texture(emissiveTexture, uv).rgb * 0.30;
    glow += texture(emissiveTexture, uv + vec2(texel.x, 0)).rgb * 0.11;
    glow += texture(emissiveTexture, uv - vec2(texel.x, 0)).rgb * 0.11;
    glow += texture(emissiveTexture, uv + vec2(0, texel.y)).rgb * 0.11;
    glow += texture(emissiveTexture, uv - vec2(0, texel.y)).rgb * 0.11;
    glow += texture(emissiveTexture, uv + texel).rgb * 0.065;
    glow += texture(emissiveTexture, uv - texel).rgb * 0.065;
    glow += texture(emissiveTexture, uv + vec2(texel.x, -texel.y)).rgb * 0.065;
    glow += texture(emissiveTexture, uv + vec2(-texel.x, texel.y)).rgb * 0.065;

    vec3 scene = world + glow * settings.bloomVignetteSaturation.x;
    scene *= settings.gradeExposure.rgb * settings.gradeExposure.a;
    float luma = dot(scene, vec3(0.2126, 0.7152, 0.0722));
    scene = mix(vec3(luma), scene, settings.bloomVignetteSaturation.w);
    vec2 centered = uv * 2.0 - 1.0;
    float vignette = smoothstep(1.28, 0.35, dot(centered, centered));
    scene *= mix(1.0 - settings.bloomVignetteSaturation.z, 1.0, vignette);
    vec3 finalSrgb = linearToSrgb(toneMap(scene));

    vec4 overlay = texture(overlayTexture, uv);
    vec3 overlaySrgb = linearToSrgb(overlay.rgb);
    finalSrgb = mix(finalSrgb, overlaySrgb, overlay.a);
    outColor = vec4(finalSrgb, 1.0) * color;
}
