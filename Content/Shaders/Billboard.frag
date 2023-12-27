#version 450

layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec4 out_Color;

layout (binding = 1) uniform sampler2D image;

void main() {
	out_Color = texture(image, in_uv) * vec4(in_color, 1.0);
}