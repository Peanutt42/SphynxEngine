use std::marker::PhantomData;
use wgpu::{util::{BufferInitDescriptor, DeviceExt}, BindGroup, BindGroupDescriptor, BindGroupEntry, BindGroupLayout, BindGroupLayoutDescriptor, BindGroupLayoutEntry, BindingType, Buffer, BufferBindingType, BufferUsages, Device, Queue, RenderPass, ShaderStages};


pub struct UniformBuffer<T: bytemuck::Pod + bytemuck::Zeroable> {
	buffer: Buffer,
	bind_group: BindGroup,
	bind_group_layout: BindGroupLayout,
	phantom: PhantomData<T>,
}

impl<T> UniformBuffer<T> where T: bytemuck::Pod + bytemuck::Zeroable {
	#[allow(dead_code)]
	pub fn for_vertex(data: T, label: Option<&str>, device: &Device) -> Self {
		Self::new(data, true, false, label, device)
	}
	pub fn for_fragment(data: T, label: Option<&str>, device: &Device) -> Self {
		Self::new(data, false, true, label, device)
	}
	pub fn for_vertex_and_fragment(data: T, label: Option<&str>, device: &Device) -> Self {
		Self::new(data, true, true, label, device)
	}

	pub fn new(data: T, vertex: bool, fragment: bool, label: Option<&str>, device: &Device) -> Self {
		let buffer = device.create_buffer_init(
			&BufferInitDescriptor {
				label,
				contents: bytemuck::cast_slice(&[data]),
				usage: BufferUsages::UNIFORM | BufferUsages::COPY_DST,
			}
		);

		let mut visibility = ShaderStages::empty();
		if vertex {
			visibility |= ShaderStages::VERTEX;
		}
		if fragment {
			visibility |= ShaderStages::FRAGMENT;
		}

		let bind_group_layout = device.create_bind_group_layout(&BindGroupLayoutDescriptor {
			entries: &[
				BindGroupLayoutEntry {
					binding: 0,
					visibility,
					ty: BindingType::Buffer {
						ty: BufferBindingType::Uniform,
						has_dynamic_offset: false,
						min_binding_size: None,
					},
					count: None,
				}
			],
			label: None,
		});

		let bind_group = device.create_bind_group(&BindGroupDescriptor {
			layout: &bind_group_layout,
			entries: &[
				BindGroupEntry {
					binding: 0,
					resource: buffer.as_entire_binding(),
				}
			],
			label: None,
		});

		Self {
			buffer,
			bind_group,
			bind_group_layout,
			phantom: PhantomData,
		}
	}

	pub fn get_binding(&self) -> &BindGroupLayout {
		&self.bind_group_layout
	}

	pub fn update(&self, data: T, queue: &Queue) {
		queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(&[data]));
	}

	pub fn bind<'a>(&'a self, bind_group: u32, renderpass: &'_ mut RenderPass<'a>) {
		renderpass.set_bind_group(bind_group, &self.bind_group, &[]);
	}
}
