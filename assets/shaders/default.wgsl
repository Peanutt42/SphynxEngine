const PI = 3.14159265359;

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

    @location(9) rotation_matrix_0: vec3<f32>,
    @location(10) rotation_matrix_1: vec3<f32>,
    @location(11) rotation_matrix_2: vec3<f32>,
};

struct Camera {
	proj_view: mat4x4<f32>,
	position: vec3<f32>,
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
		instance.model_matrix_3
	);

	let rotation_matrix = mat3x3<f32>(
		instance.rotation_matrix_0,
		instance.rotation_matrix_1,
		instance.rotation_matrix_2
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
	let metalic = 0.4;//0.8;
	let roughness = 0.25;//0.2;

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use f0
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
	let f0 = mix(vec3<f32>(0.04), input.color, metalic);

	// for multiple lights, start at 0.0 and add all together
	let lo = light_equation(f0, light, camera.position, input.world_position, input.color, input.world_normal, metalic, roughness);

	// ambient lighting (note that the next IBL tutorial will replace
	// this ambient lighting with environment lighting).
	let ambient = vec3<f32>(0.03) * input.color;

	var color = ambient + lo;

	// HDR tonemapping
	color = color / (color + vec3<f32>(1.0));
	// gamma correct
	color = pow(color, vec3<f32>(1.0/2.2));

	return vec4<f32>(color, 1.0);
}

fn light_equation(f0: vec3<f32>, light: Light, cam_pos: vec3<f32>, frag_pos: vec3<f32>, frag_color: vec3<f32>, frag_normal: vec3<f32>, metalic: f32, roughness: f32) -> vec3<f32> {
	// calculate per-light radiance
	let n = normalize(frag_normal);
	let v = normalize(cam_pos - frag_pos);
	let l = normalize(light.position - frag_pos);
	let h = normalize(v + l);
	let distance = length(l);
	let attenuation = 1.0 / (distance * distance);
	let radiance = light.color * attenuation;

	// Cook-Torrance BRDF
	let ndf = distribution_ggx(n, h, roughness);
	let g = geometry_smith(n, v, l, roughness);
	let f = fresnel_schlick(clamp(dot(h, v), 0.0, 1.0), f0);

	let numerator    = ndf * g * f;
	let denominator = 4.0 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
	let specular = numerator / denominator;

	// k_s is equal to Fresnel
	let k_s = f;
	// for energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (k_d) should equal 1.0 - k_s.
	var k_d = vec3<f32>(1.0) - k_s;
	// multiply k_d by the inverse metalness such that only non-metals
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	k_d *= 1.0 - metalic;

	// scale light by n_dot_l
	let n_dot_l = max(dot(n, l), 0.0);

	// add to outgoing radiance Lo
	return (k_d * frag_color / PI + specular) * radiance * n_dot_l;  // note that we already multiplied the BRDF by the Fresnel (k_s) so we won't multiply by k_s again
}

fn distribution_ggx(n: vec3<f32>, h: vec3<f32>, roughness: f32) -> f32 {
	let a = roughness * roughness;
	let a2 = a * a;
	let n_dot_h = max(dot(n, h), 0.0);
	let n_dot_h2 = n_dot_h * n_dot_h;

	let nom = a2;
	var denom = (n_dot_h2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

fn geometry_schlick_ggx(n_dot_v: f32, roughness: f32) -> f32 {
	let r = (roughness + 1.0);
	let k = (r * r) / 8.0;

	let nom = n_dot_v;
	let denom = n_dot_v * (1.0 - k) + k;

	return nom / denom;
}

fn geometry_smith(n: vec3<f32>, v: vec3<f32>, l: vec3<f32>, roughness: f32) -> f32 {
	let n_dot_v = max(dot(n, v), 0.0);
	let n_dot_l = max(dot(n, l), 0.0);
	let ggx2 = geometry_schlick_ggx(n_dot_v, roughness);
	let ggx1 = geometry_schlick_ggx(n_dot_l, roughness);
	return ggx1 * ggx2;
}

fn fresnel_schlick(cos_theta: f32, f0: vec3<f32>) -> vec3<f32> {
	return f0 + (1.0 - f0) * pow(clamp(1.0 - cos_theta, 0.0, 1.0), 5.0);
}