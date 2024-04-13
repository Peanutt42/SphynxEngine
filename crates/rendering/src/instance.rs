use cgmath::Matrix4;
use wgpu::util::DeviceExt;

pub trait InstanceData : bytemuck::Pod + bytemuck::Zeroable {
	fn desc() -> wgpu::VertexBufferLayout<'static>;
}

#[repr(C)]
#[derive(Copy, Clone, bytemuck::Pod, bytemuck::Zeroable)]
pub struct Model_InstanceData {
	pub model: [[f32; 4]; 4],
}

impl Model_InstanceData {
	pub fn new(model: Matrix4<f32>) -> Self {
		Self {
			model: (model).into(),
		}
	}

	const ATTRIBS: [wgpu::VertexAttribute; 4] = wgpu::vertex_attr_array![
		5 => Float32x4,
		6 => Float32x4,
		7 => Float32x4,
		8 => Float32x4
	];
}

impl InstanceData for Model_InstanceData {
	fn desc() -> wgpu::VertexBufferLayout<'static> {
		use std::mem;
		wgpu::VertexBufferLayout {
			array_stride: mem::size_of::<Self>() as wgpu::BufferAddress,
			step_mode: wgpu::VertexStepMode::Instance,
			attributes: &Self::ATTRIBS,
		}
	}
}


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
