struct VertexInput {
    @location(0) position: vec3<f32>,
    @location(1) color: vec3<f32>,
    @location(2) normal: vec3<f32>,
};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) color: vec3<f32>,
	@location(1) world_position: vec3<f32>,
	@location(2) world_normal: vec3<f32>,
}

struct InstanceInput {
    @location(5) model_matrix_0: vec4<f32>,
    @location(6) model_matrix_1: vec4<f32>,
    @location(7) model_matrix_2: vec4<f32>,
    @location(8) model_matrix_3: vec4<f32>,
};

struct Camera {
	proj_view: mat4x4<f32>,
}
@group(0) @binding(0)
var<uniform> camera: Camera;

struct Light {
	position: vec3<f32>,
	color: vec3<f32>,
}
@group(1) @binding(0)
var<uniform> light: Light;


@vertex
fn vs_main(vertex: VertexInput, instance: InstanceInput) -> VertexOutput {
	let model_matrix = mat4x4<f32>(
        instance.model_matrix_0,
        instance.model_matrix_1,
        instance.model_matrix_2,
        instance.model_matrix_3,
    );

	let world_position = model_matrix * vec4<f32>(vertex.position, 1.0);
	//let model_rotation_matrix = mat3x3<f32>(model_matrix[0].xyz, model_matrix[1].xyz, model_matrix[2].xyz);
	//let world_normal = model_rotation_matrix * vertex.normal;

	var output = VertexOutput();
	output.position = camera.proj_view * world_position;
	output.color = vertex.color;
	output.world_position = world_position.xyz;
	output.world_normal = vertex.normal;//world_normal;
	return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
	let light_dir = normalize(light.position - input.world_position);

	let diffuse_strength = max(dot(input.world_normal, light_dir), 0.0);
	let diffuse_color = light.color * diffuse_strength;

	let ambient_strength = 0.03;
	let ambient_color = light.color * ambient_strength;

	return vec4<f32>((ambient_color + diffuse_color) * input.color, 1.0);
}
