use wgpu::{util::{BufferInitDescriptor, DeviceExt}, Buffer, BufferUsages, Device, IndexFormat, RenderPass};
use crate::instance_buffer::InstanceBuffer;
use crate::instance_data::InstanceData;
use crate::vertex::Vertex;
use crate::shader::VERTEX_BUFFER_BIND_SLOT;

pub struct Mesh {
	vertex_buffer: Buffer,
	vertex_count: u32,
	index_buffer: Option<Buffer>,
}

impl Mesh {
	pub fn with_vertices<V: Vertex>(vertices: &[V], indices: Option<&[u32]>, device: &Device) -> Self {
		Self::new(bytemuck::cast_slice(vertices), vertices.len() as u32, indices, device)
	}

	pub fn new(vertices: &[u8], vertex_count: u32, indices: Option<&[u32]>, device: &Device) -> Self {
		let vertex_buffer = device.create_buffer_init(
			&BufferInitDescriptor {
				label: Some("Vertex Buffer"),
				contents: vertices,
				usage: BufferUsages::VERTEX,
			}
		);

		let index_buffer = indices.map(|indicies| {
			device.create_buffer_init(
				&BufferInitDescriptor {
					label: Some("Index Buffer"),
					contents: bytemuck::cast_slice(indicies),
					usage: BufferUsages::INDEX,
				}
			)}
		);

		Self {
			vertex_buffer,
			vertex_count,
			index_buffer,
		}
	}

	pub fn draw<'a>(&'a self, renderpass: &'_ mut RenderPass<'a>) {
		self.bind(renderpass);
		renderpass.draw(0..self.vertex_count, 0..1);
	}

	pub fn draw_instanced<'a, I: InstanceData>(&'a self, instance_buffer: &'a InstanceBuffer<I>, renderpass: &'_ mut RenderPass<'a>) {
		self.bind(renderpass);
		instance_buffer.bind(renderpass);
		renderpass.draw(0..self.vertex_count, 0..instance_buffer.instances.len() as u32);
	}

	fn bind<'a>(&'a self, renderpass: &'_ mut RenderPass<'a>) {
		renderpass.set_vertex_buffer(VERTEX_BUFFER_BIND_SLOT, self.vertex_buffer.slice(..));
		if let Some(index_buffer) = &self.index_buffer {
			renderpass.set_index_buffer(index_buffer.slice(..), IndexFormat::Uint32);
		}
	}
}
