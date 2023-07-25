#type vertex
#version 450

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_UV;

layout(location = 0) out vec3 out_FragColor;

void main() {
    gl_Position = vec4(in_Position, 1.0);
    out_FragColor = vec3(1, 0 , 0);
}


#type fragment
#version 450

layout(location = 0) in vec3 in_FragColor;

layout(location = 0) out vec4 out_Color;

void main() {
    out_Color = vec4(in_FragColor, 1.0);
}