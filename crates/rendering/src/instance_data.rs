use cgmath::Matrix4;
use sphynx_derive_vertex_attrib::InstanceData;

pub trait InstanceData : bytemuck::Pod + bytemuck::Zeroable {
	fn desc() -> wgpu::VertexBufferLayout<'static>;
}

#[repr(C)]
#[derive(Copy, Clone, bytemuck::Pod, bytemuck::Zeroable, InstanceData)]
pub struct Model_InstanceData {
	pub model: [[f32; 4]; 4],
}

impl Model_InstanceData {
	pub fn new(model: Matrix4<f32>) -> Self {
		Self {
			model: (model).into(),
		}
	}

	const ATTRIBS: [wgpu::VertexAttribute; 4] = wgpu::vertex_attr_array![
		5 => Float32x4,
		6 => Float32x4,
		7 => Float32x4,
		8 => Float32x4
	];
}
