#version 330 core

out vec4 FragColor;

in vec2 uv;
in vec3 color;

uniform sampler2D billboard;

void main()
{
    FragColor = texture(billboard, uv) * vec4(color, 1.0);
}