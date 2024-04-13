pub trait Vertex : bytemuck::Pod + bytemuck::Zeroable {
	fn desc() -> wgpu::VertexBufferLayout<'static>;
}

#[repr(C)]
#[derive(Copy, Clone, Debug, bytemuck::Pod, bytemuck::Zeroable)]
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

impl Vertex for PC_Vertex {
	fn desc() -> wgpu::VertexBufferLayout<'static> {
		wgpu::VertexBufferLayout {
			array_stride: std::mem::size_of::<Self>() as wgpu::BufferAddress,
			step_mode: wgpu::VertexStepMode::Vertex,
			attributes: &Self::ATTRIBS,
		}
	}
}