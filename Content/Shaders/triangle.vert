#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aUV;

out vec3 f_color;
out vec2 f_uv;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    f_color = aColor;
    f_uv = aUV;
}