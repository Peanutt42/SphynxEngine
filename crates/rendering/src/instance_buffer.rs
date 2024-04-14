use wgpu::util::DeviceExt;
use crate::instance_data::InstanceData;
use crate::shader::INSTANCE_BUFFER_BIND_SLOT;

pub struct InstanceBuffer<I: InstanceData> {
	pub instances: Vec<I>,
	buffer: wgpu::Buffer,
}

impl<I: InstanceData> InstanceBuffer<I> {
	pub fn new(instances: Vec<I>, device: &wgpu::Device) -> Self {
		let buffer = device.create_buffer_init(
			&wgpu::util::BufferInitDescriptor {
				label: Some("Instance Buffer"),
				contents: bytemuck::cast_slice(&instances),
				usage: wgpu::BufferUsages::VERTEX | wgpu::BufferUsages::COPY_DST,
			}
		);

		Self {
			instances,
			buffer,
		}
	}

	pub fn update(&self, queue: &wgpu::Queue) {
		queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(&self.instances));
	}

	pub fn bind<'a, 'b>(&'a self, renderpass: &'b mut wgpu::RenderPass<'a>) {
		renderpass.set_vertex_buffer(INSTANCE_BUFFER_BIND_SLOT, self.buffer.slice(..));
	}
}
