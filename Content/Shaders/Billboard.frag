#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D billboard;

void main()
{
    FragColor = texture(billboard, uv);
}