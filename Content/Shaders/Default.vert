#version 450

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;
layout(location = 3) in mat4 in_InstanceModel;

layout(location = 0) out vec3 out_FragColor;

layout(binding = 0) uniform UniformBufferObject {
    mat4 proj_view; // proj * view
} v_ubo;

void main() {
    gl_Position = v_ubo.proj_view * in_InstanceModel * vec4(in_Position, 1.0);
    out_FragColor = vec3(1, 0 , 0);
}