use wgpu::{Buffer, BufferUsages, Device, Queue, RenderPass, util::{DeviceExt, BufferInitDescriptor}};
use crate::instance_data::InstanceData;
use crate::shader::INSTANCE_BUFFER_BIND_SLOT;

pub struct InstanceBuffer<I: InstanceData> {
	pub instances: Vec<I>,
	buffer: Buffer,
}

impl<I: InstanceData> InstanceBuffer<I> {
	pub fn new(instances: Vec<I>, device: &Device) -> Self {
		let buffer = device.create_buffer_init(
			&BufferInitDescriptor {
				label: Some("Instance Buffer"),
				contents: bytemuck::cast_slice(&instances),
				usage: BufferUsages::VERTEX | BufferUsages::COPY_DST,
			}
		);

		Self {
			instances,
			buffer,
		}
	}

	pub fn update(&self, queue: &Queue) {
		queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(&self.instances));
	}

	pub fn bind<'a>(&'a self, renderpass: &'_ mut RenderPass<'a>) {
		renderpass.set_vertex_buffer(INSTANCE_BUFFER_BIND_SLOT, self.buffer.slice(..));
	}
}
