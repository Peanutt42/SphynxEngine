#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D screen;

void main() {
    outColor = texture(screen, vec2(texCoord.x, 1 - texCoord.y));
}