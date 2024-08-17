use std::marker::PhantomData;

use wgpu::util::DeviceExt;

pub struct UniformBuffer<T: bytemuck::Pod + bytemuck::Zeroable> {
	buffer: wgpu::Buffer,
	bind_group: wgpu::BindGroup,
	bind_group_layout: wgpu::BindGroupLayout,
	phantom: PhantomData<T>,
}

impl<T> UniformBuffer<T> where T: bytemuck::Pod + bytemuck::Zeroable {
	pub fn for_vertex(data: T, label: Option<&str>, device: &wgpu::Device) -> Self {
		Self::new(data, true, false, label, device)
	}
	pub fn for_fragment(data: T, label: Option<&str>, device: &wgpu::Device) -> Self {
		Self::new(data, false, true, label, device)
	}
	#[allow(dead_code)]
	pub fn for_vertex_and_fragment(data: T, label: Option<&str>, device: &wgpu::Device) -> Self {
		Self::new(data, true, true, label, device)
	}

	pub fn new(data: T, vertex: bool, fragment: bool, label: Option<&str>, device: &wgpu::Device) -> Self {
		let buffer = device.create_buffer_init(
			&wgpu::util::BufferInitDescriptor {
				label,
				contents: bytemuck::cast_slice(&[data]),
				usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
			}
		);

		let mut visibility = wgpu::ShaderStages::empty();
		if vertex {
			visibility |= wgpu::ShaderStages::VERTEX;
		}
		if fragment {
			visibility |= wgpu::ShaderStages::FRAGMENT;
		}

		let bind_group_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
			entries: &[
				wgpu::BindGroupLayoutEntry {
					binding: 0,
					visibility,
					ty: wgpu::BindingType::Buffer {
						ty: wgpu::BufferBindingType::Uniform,
						has_dynamic_offset: false,
						min_binding_size: None,
					},
					count: None,
				}
			],
			label: None,
		});

		let bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
			layout: &bind_group_layout,
			entries: &[
				wgpu::BindGroupEntry {
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

	pub fn get_binding(&self) -> &wgpu::BindGroupLayout {
		&self.bind_group_layout
	}

	pub fn update(&self, data: T, queue: &wgpu::Queue) {
		queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(&[data]));
	}

	pub fn bind<'a>(&'a self, bind_group: u32, renderpass: &'_ mut wgpu::RenderPass<'a>) {
		renderpass.set_bind_group(bind_group, &self.bind_group, &[]);
	}
}
