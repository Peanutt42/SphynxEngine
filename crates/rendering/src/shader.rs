use std::borrow::Cow;
use wgpu::DepthStencilState;

use crate::vertex::Vertex;
use crate::instance::InstanceData;
use crate::Texture;

pub const VERTEX_BUFFER_BIND_SLOT: u32 = 0;
pub const INSTANCE_BUFFER_BIND_SLOT: u32 = 1;

#[macro_export]
macro_rules! include_shader {
	($filepath:expr, $device:expr, $swapchain_format:expr) => {{
		Shader::from_str_instanced::<$crate::vertex::PC_Vertex, $crate::instance::Model_InstanceData>(include_str!($filepath), Some($filepath), $device, $swapchain_format)
	}};
}

#[macro_export]
macro_rules! include_custom_shader {
	($vertex:expr, $instance:expr, $filepath:expr, $device:expr, $swapchain_format:expr) => {{
		Shader::from_str_instanced::<$vertex, $instance>(include_str!($filepath), Some($filepath), $device, $swapchain_format)
	}};
}

pub struct Shader {
	pub pipeline: wgpu::RenderPipeline,
}

impl Shader {
	pub fn from_str_instanced<V: Vertex, I: InstanceData>(shader_code: &str, label: Option<&str>, device: &wgpu::Device, swapchain_format: wgpu::TextureFormat) -> Self {
		Self::from_str::<V>(shader_code, Some(I::desc()), label, device, swapchain_format)
	}
	
	pub fn from_str<V: Vertex>(shader_code: &str, instance_layout: Option<wgpu::VertexBufferLayout<'static>>, label: Option<&str>, device: &wgpu::Device, swapchain_format: wgpu::TextureFormat) -> Self {
		Self::new(wgpu::ShaderSource::Wgsl(Cow::Borrowed(shader_code)), V::desc(), instance_layout, label, device, swapchain_format)
	}

	pub fn new(source: wgpu::ShaderSource, vertex_layout: wgpu::VertexBufferLayout<'static>, instance_layout: Option<wgpu::VertexBufferLayout<'static>>, label: Option<&str>, device: &wgpu::Device, swapchain_format: wgpu::TextureFormat) -> Self {
		let shader_module = device.create_shader_module(wgpu::ShaderModuleDescriptor {
			label: None,
			source,
		});

		let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
			label: None,
			bind_group_layouts: &[],
			push_constant_ranges: &[],
		});

		let mut vertex_buffers = vec![vertex_layout];
		if let Some(instance_layout) = instance_layout {
			vertex_buffers.push(instance_layout);
		}

		let pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
			label,
			layout: Some(&pipeline_layout),
			vertex: wgpu::VertexState {
				module: &shader_module,
				entry_point: "vs_main",
				buffers: &vertex_buffers,
			},
			fragment: Some(wgpu::FragmentState {
				module: &shader_module,
				entry_point: "fs_main",
				targets: &[Some(swapchain_format.into())],
			}),
			primitive: wgpu::PrimitiveState::default(),
			depth_stencil: Some(DepthStencilState {
				format: Texture::DEPTH_FORMAT,
				depth_write_enabled: true,
				depth_compare: wgpu::CompareFunction::Less,
				stencil: wgpu::StencilState::default(),
				bias: wgpu::DepthBiasState::default(),
			}),
			multisample: wgpu::MultisampleState::default(),
			multiview: None,
		});

		Self {
			pipeline,
		}
	}
}