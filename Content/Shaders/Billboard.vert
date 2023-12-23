#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
// per instance
layout (location = 2) in vec3 InstanceColor;
layout (location = 3) in mat4 InstanceModel;

out vec2 uv;
out vec3 color;

uniform mat4 proj_view;

void main()
{
    gl_Position = proj_view * InstanceModel * vec4(aPos, 1.0);
    uv = aUV;
    color = InstanceColor;
}