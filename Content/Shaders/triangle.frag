#version 330 core
out vec4 FragColor;

in vec3 f_color;
in vec2 f_uv;
uniform sampler2D a_texture;

void main()
{
    FragColor = texture(a_texture, f_uv);
}