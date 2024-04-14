struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) color: vec3<f32>,
}

struct InstanceInput {
    @location(5) model_matrix_0: vec4<f32>,
    @location(6) model_matrix_1: vec4<f32>,
    @location(7) model_matrix_2: vec4<f32>,
    @location(8) model_matrix_3: vec4<f32>,
};

struct CameraUniform {
	proj_view: mat4x4<f32>,
}

@group(0) @binding(0)
var<uniform> camera: CameraUniform;

@vertex
fn vs_main(vertex: VertexInput, instance: InstanceInput) -> VertexOutput {
	let model_matrix = mat4x4<f32>(
        instance.model_matrix_0,
        instance.model_matrix_1,
        instance.model_matrix_2,
        instance.model_matrix_3,
    );

	var output = VertexOutput();
	output.position = camera.proj_view * model_matrix * vec4<f32>(vertex.position, 1.0);
	output.color = vertex.color;
	return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
	return vec4<f32>(input.color, 1.0);
}
