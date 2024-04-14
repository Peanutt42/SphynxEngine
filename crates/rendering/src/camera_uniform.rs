use wgpu::util::DeviceExt;
use crate::shader::CAMERA_UNIFORM_BINDING;

#[repr(C)]
#[derive(Debug, Copy, Clone, bytemuck::Pod, bytemuck::Zeroable)]
pub struct CameraUniform {
	proj_view: [[f32; 4]; 4],
}

impl CameraUniform {
	pub fn new(proj_view: cgmath::Matrix4<f32>) -> Self {
		Self {
			proj_view: proj_view.into(),
		}
	}
}

pub struct CameraUniformBuffer {
	buffer: wgpu::Buffer,
	bind_group: wgpu::BindGroup,
	bind_group_layout: wgpu::BindGroupLayout,
}

impl CameraUniformBuffer {
	pub fn new(uniform: CameraUniform, device: &wgpu::Device) -> Self {
		let buffer = device.create_buffer_init(
			&wgpu::util::BufferInitDescriptor {
				label: Some("Camera Buffer"),
				contents: bytemuck::cast_slice(&[uniform]),
				usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
			}
		);

		let bind_group_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
			entries: &[
				wgpu::BindGroupLayoutEntry {
					binding: CAMERA_UNIFORM_BINDING,
					visibility: wgpu::ShaderStages::VERTEX,
					ty: wgpu::BindingType::Buffer {
						ty: wgpu::BufferBindingType::Uniform,
						has_dynamic_offset: false,
						min_binding_size: None,
					},
					count: None,
				}
			],
			label: Some("camera_bind_group_layout"),
		});

		let bind_group = device.create_bind_group(&wgpu::BindGroupDescriptor {
			layout: &bind_group_layout,
			entries: &[
				wgpu::BindGroupEntry {
					binding: CAMERA_UNIFORM_BINDING,
					resource: buffer.as_entire_binding(),
				}
			],
			label: Some("camera_bind_group"),
		});

		Self {
			buffer,
			bind_group,
			bind_group_layout,
		}
	}

	pub fn get_binding(&self) -> &wgpu::BindGroupLayout {
		&self.bind_group_layout
	}

	pub fn update(&self, uniform: CameraUniform, queue: &wgpu::Queue) {
		queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(&[uniform]));
	}

	pub fn bind<'a, 'b>(&'a self, renderpass: &'b mut wgpu::RenderPass<'a>) {
		renderpass.set_bind_group(CAMERA_UNIFORM_BINDING, &self.bind_group, &[]);
	}
}
