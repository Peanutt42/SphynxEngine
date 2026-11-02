import sphynx_engine::pcn_vertex::PCN_Vertex;
import sphynx_engine::instance::InstanceData;
import sphynx_engine::camera::Camera;
import sphynx_engine::light::{Light, light_equation};

struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) color: vec3<f32>,
	@location(1) world_position: vec3<f32>,
	@location(2) world_normal: vec3<f32>,
}

@group(0) @binding(0)
var<uniform> camera: Camera;

@group(1) @binding(0)
var<uniform> light: Light;

@vertex
fn vs_main(vertex: PCN_Vertex, instance: InstanceData) -> VertexOutput {
	let model_matrix = mat4x4<f32>(
		instance.model_matrix_0,
		instance.model_matrix_1,
		instance.model_matrix_2,
		instance.model_matrix_3,
	);

	let rotation_matrix = mat3x3<f32>(
		instance.rotation_matrix_0,
		instance.rotation_matrix_1,
		instance.rotation_matrix_2,
	);

	let world_position = model_matrix * vec4<f32>(vertex.position, 1.0);

	var output = VertexOutput();
	output.position = camera.proj_view * world_position;
	output.color = vertex.color;
	output.world_position = world_position.xyz;
	output.world_normal = rotation_matrix * vertex.normal;
	return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
	// TODO: add material system
	let metalic = 0.4; //0.8;
	let roughness = 0.25; //0.2;

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use f0
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
	let f0 = mix(vec3<f32>(0.04), input.color, metalic);

	// for multiple lights, start at 0.0 and add all together
	let lo = light_equation(
		f0,
		light,
		camera.position,
		input.world_position,
		input.color,
		input.world_normal,
		metalic,
		roughness,
	);

	// ambient lighting (note that the next IBL tutorial will replace
	// this ambient lighting with environment lighting).
	let ambient = vec3<f32>(0.03) * input.color;

	var color = ambient + lo;

	// HDR tonemapping
	color = color / (color + vec3<f32>(1.0));

	return vec4<f32>(color, 1.0);
}
