use wgpu::util::DeviceExt;
use crate::vertex::Vertex;

pub struct Mesh {
	vertex_buffer: wgpu::Buffer,
}

impl Mesh {
	pub fn with_vertices<V: Vertex + bytemuck::Pod + bytemuck::Zeroable>(vertices: &[V], device: &wgpu::Device) -> Self {
		Self::new(bytemuck::cast_slice(vertices), device)
	}

	pub fn new(vertices: &[u8], device: &wgpu::Device) -> Self {
		let vertex_buffer = device.create_buffer_init(
			&wgpu::util::BufferInitDescriptor {
				label: Some("Vertex Buffer"),
				contents: vertices,
				usage: wgpu::BufferUsages::VERTEX,
			}
		);
		
		Self {
			vertex_buffer,
		}
	}

	pub fn get_vertex_buffer_slice(&self) -> wgpu::BufferSlice<'_> {
		self.vertex_buffer.slice(..)
	}
}