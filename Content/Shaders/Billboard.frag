#version 330 core

out vec4 FragColor;

in vec2 uv;

uniform sampler2D billboard;
uniform vec3 color;

void main()
{
    FragColor = texture(billboard, uv) * vec4(color, 1.0);
}