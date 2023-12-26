#version 450

const vec2 OFFSETS[6] = vec2[](
  vec2(-0.5, -0.5),
  vec2( 0.5, -0.5),
  vec2(-0.5,  0.5),
  vec2( 0.5, -0.5),
  vec2( 0.5,  0.5),
  vec2(-0.5,  0.5)
);

const vec2 UVS[6] = vec2[](
    vec2(0,0, 0.0),
    vec2(1,0, 0.0),
    vec2(0,0, 1.0),
    vec2(1,0, 0.0),
    vec2(1,0, 1.0),
    vec2(0,0, 1.0),
);

layout (location = 0) in vec3 in_InstancePosition;

layout (locaiton = 0) out vec2 out_uv;

layout(binding = 0) uniform CameraUniformBuffer {
    mat4 Projection;
    mat4 View;
    vec3 CameraPosition;
} CameraData;

const float SIZE = 0.5;

void main() {
    vec2 fragOffset = OFFSETS[gl_VertexIndex];
    vec3 cameraRightWorld = { CameraData.View[0][0], CameraData.View[1][0], CameraData.View[2][0] }; 
    vec3 cameraUpWorld = { CameraData.View[0][1], CameraData.View[1][1], CameraData.View[2][1] };

    vec3 positionWorld = in_InstancePosition
            + SIZE * fragOffset.x * cameraRightWorld
            + SIZE * fragOffset.y * cameraUpWorld;

    gl_Position = CameraData.Projection * CameraData.View * vec4(positionWorld, 1.0);

    out_uv = UVS[gl_VertexIndex];
}