use wgpu::util::DeviceExt;
use crate::instance_data::InstanceData;

pub struct InstanceBuffer<I: InstanceData> {
	pub instances: Vec<I>,
	pub buffer: wgpu::Buffer,
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
}
