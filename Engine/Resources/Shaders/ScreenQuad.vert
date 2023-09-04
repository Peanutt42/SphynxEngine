#version 450

vec2[] positions = vec2[](
    vec2(-1, 1),
    vec2(1, 1),
    vec2(1, -1),
    vec2(1, -1),
    vec2(-1, -1),
    vec2(-1, 1)
);

layout(location = 0) out vec2 texCoord;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    texCoord = vec2((gl_Position.x + 1.0) / 2.0, 1.0 - (gl_Position.y + 1.0) / 2.0);
}