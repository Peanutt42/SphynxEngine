#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 proj_view;
uniform mat4 model_matrix;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} vs_out;

void main()
{
    vec4 frag_pos = model_matrix * vec4(aPos, 1.0);
    gl_Position = proj_view * frag_pos;
    vs_out.FragPos = frag_pos.xyz;
    vs_out.Normal = aNormal;
}