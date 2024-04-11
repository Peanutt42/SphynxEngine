struct VertexOutput {
	@builtin(position) position: vec4<f32>,
	@location(0) color: vec3<f32>,
}

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> VertexOutput {
	let x = f32(i32(in_vertex_index) - 1);
	let y = f32(i32(in_vertex_index & 1u) * 2 - 1);
	var output = VertexOutput();
	output.position = vec4<f32>(x, y, 0.0, 1.0);

	var colors: array<vec3<f32>, 3> = array<vec3<f32>, 3>(
		vec3<f32>(1.0, 0.0, 0.0),
		vec3<f32>(0.0, 1.0, 0.0),
		vec3<f32>(0.0, 0.0, 1.0),
	);
	output.color = colors[in_vertex_index];

	return output;
}

@fragment
fn fs_main(input: VertexOutput) -> @location(0) vec4<f32> {
	return vec4<f32>(input.color, 1.0);
}
