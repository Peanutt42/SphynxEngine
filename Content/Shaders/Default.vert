#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
// per instance
layout (location = 2) in vec3 Albedo;
layout (location = 3) in float Metalic;
layout (location = 4) in float Roughness;
layout (location = 5) in mat4 InstanceModel;

layout (std140) uniform CameraData {
    mat4 ProjView;
    vec3 CameraPosition;
 };

out VS_OUT {
    vec3 FragPos;
    vec3 CameraPosition;
    vec3 Normal;
    vec3 Albedo;
    float Metalic;
    float Roughness;
} vs_out;

void main()
{
    vec4 frag_pos = InstanceModel * vec4(aPos, 1.0);
    gl_Position = ProjView * frag_pos;
    vs_out.FragPos = frag_pos.xyz;
    vs_out.CameraPosition = CameraPosition;
    vs_out.Normal = aNormal;
    vs_out.Albedo = Albedo;
    vs_out.Metalic = Metalic;
    vs_out.Roughness = Roughness;
}