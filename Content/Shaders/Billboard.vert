#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

out vec2 uv;

uniform mat4 proj_view;
uniform mat4 model_matrix;

void main()
{
    gl_Position = proj_view * model_matrix * vec4(aPos, 1.0);
    uv = aUV;
}