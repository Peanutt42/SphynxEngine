use std::borrow::Cow;
use crate::vertex::Vertex;
use crate::instance::InstanceData;

pub const VERTEX_BUFFER_BIND_SLOT: u32 = 0;
pub const INSTANCE_BUFFER_BIND_SLOT: u32 = 1;

pub struct Shader {
	pipeline: wgpu::RenderPipeline,
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
			depth_stencil: None,
			multisample: wgpu::MultisampleState::default(),
			multiview: None,
		});

		Self {
			pipeline,
		}
	}

	pub fn get_pipeline(&self) -> &wgpu::RenderPipeline {
		&self.pipeline
	}
}