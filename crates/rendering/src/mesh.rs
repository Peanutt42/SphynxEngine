use wgpu::util::DeviceExt;
use crate::instance_buffer::InstanceBuffer;
use crate::instance_data::InstanceData;
use crate::vertex::Vertex;
use crate::shader::VERTEX_BUFFER_BIND_SLOT;

pub struct Mesh {
	vertex_buffer: wgpu::Buffer,
	vertex_count: u32,
}

impl Mesh {
	pub fn with_vertices<V: Vertex>(vertices: &[V], device: &wgpu::Device) -> Self {
		Self::new(bytemuck::cast_slice(vertices), vertices.len() as u32, device)
	}

	pub fn new(vertices: &[u8], vertex_count: u32, device: &wgpu::Device) -> Self {
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

	pub fn draw<'a, 'b>(&'a self, renderpass: &'b mut wgpu::RenderPass<'a>) {
		self.bind(renderpass);
		renderpass.draw(0..self.vertex_count, 0..1);
	}

	pub fn draw_instanced<'a, 'b, I: InstanceData>(&'a self, instance_buffer: &'a InstanceBuffer<I>, renderpass: &'b mut wgpu::RenderPass<'a>) {
		self.bind(renderpass);
		instance_buffer.bind(renderpass);
		renderpass.draw(0..self.vertex_count, 0..instance_buffer.instances.len() as u32);
	}

	fn bind<'a, 'b>(&'a self, renderpass: &'b mut wgpu::RenderPass<'a>) {
		renderpass.set_vertex_buffer(VERTEX_BUFFER_BIND_SLOT, self.vertex_buffer.slice(..));
	}
}
