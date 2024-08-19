use wgpu::{VertexAttribute, VertexBufferLayout, vertex_attr_array};
use sphynx_derive_vertex_attrib::Vertex;

pub trait Vertex : bytemuck::Pod + bytemuck::Zeroable {
	fn desc() -> VertexBufferLayout<'static>;
}

#[repr(C)]
#[derive(Copy, Clone, Debug, bytemuck::Pod, bytemuck::Zeroable, Vertex)]
#[allow(non_camel_case_types)]
pub struct PCN_Vertex {
	pub position: [f32; 3],
	pub color: [f32; 3],
	pub normal: [f32; 3],
}

impl PCN_Vertex {
	const ATTRIBS: [VertexAttribute; 3] = vertex_attr_array![
		0 => Float32x3,
		1 => Float32x3,
		2 => Float32x3
	];
}
