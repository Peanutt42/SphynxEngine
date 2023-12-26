#version 450

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

// per instance
layout(location = 3) in vec3 in_InstanceAlbedo;
layout(location = 4) in float in_InstanceMetalic;
layout(location = 5) in float in_InstanceRoughness;
layout(location = 6) in mat4 in_InstanceModel;

layout(location = 0) out VS_OUT {
    vec3 FragPos;
    vec3 CameraPosition;
    vec3 Normal;
    vec3 Albedo;
    float Metalic;
    float Roughness;
} vs_out;

layout(binding = 0) uniform CameraUniformBuffer {
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} CameraData;

void main() {
    vec4 frag_pos = in_InstanceModel * vec4(in_Position, 1.0);
    gl_Position = CameraData.Projection * CameraData.View * frag_pos;

    vs_out.FragPos = frag_pos.xyz;
    vs_out.CameraPosition = CameraData.CameraPosition;
    vs_out.Normal = in_Normal;
    vs_out.Albedo = in_InstanceAlbedo;
    vs_out.Metalic = in_InstanceMetalic;
    vs_out.Roughness = in_InstanceRoughness;
}