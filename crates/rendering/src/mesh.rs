use wgpu::util::DeviceExt;
use crate::vertex::Vertex;

pub struct Mesh {
	pub vertex_buffer: wgpu::Buffer,
	pub vertex_count: usize,
}

impl Mesh {
	pub fn with_vertices<V: Vertex>(vertices: &[V], device: &wgpu::Device) -> Self {
		Self::new(bytemuck::cast_slice(vertices), vertices.len(), device)
	}

	pub fn new(vertices: &[u8], vertex_count: usize, device: &wgpu::Device) -> Self {
		let vertex_buffer = device.create_buffer_init(
			&wgpu::util::BufferInitDescriptor {
				label: Some("Vertex Buffer"),
				contents: vertices,
				usage: wgpu::BufferUsages::VERTEX,
			}
		);
		
		Self {
			vertex_buffer,
			vertex_count,
		}
	}
}