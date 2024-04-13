use sphynx_derive_vertex_attrib::Vertex;

pub trait Vertex : bytemuck::Pod + bytemuck::Zeroable {
	fn desc() -> wgpu::VertexBufferLayout<'static>;
}

#[repr(C)]
#[derive(Copy, Clone, Debug, bytemuck::Pod, bytemuck::Zeroable, Vertex)]
pub struct PC_Vertex {
	pub position: [f32; 3],
	pub color: [f32; 3],
}

impl PC_Vertex {
	const ATTRIBS: [wgpu::VertexAttribute; 2] = wgpu::vertex_attr_array![
		0 => Float32x3,
		1 => Float32x3
	];
}
