use cgmath::{Matrix4, Matrix3};
use wgpu::{VertexAttribute, VertexBufferLayout, vertex_attr_array};
use sphynx_derive_vertex_attrib::InstanceData;

pub trait InstanceData : bytemuck::Pod + bytemuck::Zeroable {
	fn desc() -> VertexBufferLayout<'static>;
}

#[repr(C)]
#[derive(Copy, Clone, bytemuck::Pod, bytemuck::Zeroable, InstanceData)]
#[allow(non_camel_case_types)]
pub struct Model_InstanceData {
	pub model: [[f32; 4]; 4],
	pub rotation: [[f32; 3]; 3],
}

impl Model_InstanceData {
	pub fn new(model: Matrix4<f32>, rotation: Matrix3<f32>) -> Self {
		Self {
			model: model.into(),
			rotation: rotation.into(),
		}
	}

	const ATTRIBS: [VertexAttribute; 7] = vertex_attr_array![
		5  => Float32x4,
		6  => Float32x4,
		7  => Float32x4,
		8  => Float32x4,
		9  => Float32x3,
		10 => Float32x3,
		11 => Float32x3
	];
}
