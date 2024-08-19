use std::borrow::Cow;
use wgpu::{BindGroupLayout, CompareFunction, DepthBiasState, DepthStencilState, Device, Face, FragmentState, MultisampleState, PipelineLayoutDescriptor, PrimitiveState, RenderPass, RenderPipeline, RenderPipelineDescriptor, ShaderModuleDescriptor, ShaderSource, StencilState, TextureFormat, VertexBufferLayout, VertexState};
use crate::vertex::Vertex;
use crate::instance_data::InstanceData;
use crate::depth_texture::DepthTexture;

pub const VERTEX_BUFFER_BIND_SLOT: u32 = 0;
pub const INSTANCE_BUFFER_BIND_SLOT: u32 = 1;
pub const CAMERA_UNIFORM_BIND_GROUP: u32 = 0;
pub const LIGHT_UNIFORM_BIND_GROUP: u32 = 1;

#[macro_export]
macro_rules! include_shader {
	($filepath:expr, $bindings:expr, $device:expr, $swapchain_format:expr) => {{
		Shader::from_str_instanced::<$crate::vertex::PCN_Vertex, $crate::instance_data::Model_InstanceData>(include_str!($filepath), Some($filepath), $bindings, $device, $swapchain_format)
	}};
}

#[macro_export]
macro_rules! include_custom_shader {
	($vertex:expr, $instance:expr, $filepath:expr, $bindings:expr, $device:expr, $swapchain_format:expr) => {{
		Shader::from_str_instanced::<$vertex, $instance>(include_str!($filepath), Some($filepath), $bindings, $device, $swapchain_format)
	}};
}

pub struct Shader {
	pipeline: RenderPipeline,
}

impl Shader {
	pub fn from_str_instanced<V: Vertex, I: InstanceData>(shader_code: &str, label: Option<&str>, bindings: &[&BindGroupLayout], device: &Device, swapchain_format: TextureFormat) -> Self {
		Self::from_str::<V>(shader_code, Some(I::desc()), label, bindings, device, swapchain_format)
	}

	pub fn from_str<V: Vertex>(shader_code: &str, instance_layout: Option<VertexBufferLayout<'static>>, label: Option<&str>, bindings: &[&BindGroupLayout], device: &Device, swapchain_format: TextureFormat) -> Self {
		Self::new(ShaderSource::Wgsl(Cow::Borrowed(shader_code)), V::desc(), instance_layout, label, bindings, device, swapchain_format)
	}

	pub fn new(source: ShaderSource, vertex_layout: VertexBufferLayout<'static>, instance_layout: Option<VertexBufferLayout<'static>>, label: Option<&str>, bindings: &[&BindGroupLayout], device: &Device, swapchain_format: TextureFormat) -> Self {
		let shader_module = device.create_shader_module(ShaderModuleDescriptor {
			label,
			source,
		});

		let pipeline_layout = device.create_pipeline_layout(&PipelineLayoutDescriptor {
			label: None,
			bind_group_layouts: bindings,
			push_constant_ranges: &[],
		});

		let mut vertex_buffers = vec![vertex_layout];
		if let Some(instance_layout) = instance_layout {
			vertex_buffers.push(instance_layout);
		}

		let pipeline = device.create_render_pipeline(&RenderPipelineDescriptor {
			label,
			layout: Some(&pipeline_layout),
			vertex: VertexState {
				module: &shader_module,
				entry_point: "vs_main",
				buffers: &vertex_buffers,
			},
			fragment: Some(FragmentState {
				module: &shader_module,
				entry_point: "fs_main",
				targets: &[Some(swapchain_format.into())],
			}),
			primitive: PrimitiveState {
				cull_mode: Some(Face::Back),
				..Default::default()
			},
			depth_stencil: Some(DepthStencilState {
				format: DepthTexture::FORMAT,
				depth_write_enabled: true,
				depth_compare: CompareFunction::Less,
				stencil: StencilState::default(),
				bias: DepthBiasState::default(),
			}),
			multisample: MultisampleState::default(),
			multiview: None,
		});

		Self {
			pipeline,
		}
	}

	pub fn bind<'a>(&'a self, renderpass: &'_ mut RenderPass<'a>) {
		renderpass.set_pipeline(&self.pipeline);
	}
}
