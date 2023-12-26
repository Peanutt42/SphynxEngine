#version 450

layout (location = 0) in vec2 in_uv;
layout (location = 0) out vec4 out_Color;

layout (binding = 1) uniform sampler2D image;

void main() {
	out_Color = texture(image, in_uv);
}