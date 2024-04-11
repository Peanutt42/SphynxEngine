use winit::dpi::PhysicalSize;
use winit::window::Window;
use wgpu::util::DeviceExt;
use cgmath::{Vector3, Matrix4};
use std::borrow::Cow;

pub struct Transform {
	position: Vector3<f32>,
}

impl Transform {
	pub fn new(position: Vector3<f32>) -> Self {
		Self {
			position,
		}
	}

	pub fn model_matrix(&self) -> Matrix4<f32> {
		Matrix4::from_translation(self.position)
	}
}

#[repr(C)]
#[derive(Copy, Clone, Debug, bytemuck::Pod, bytemuck::Zeroable)]
struct Vertex {
	position: [f32; 3],
	color: [f32; 3],
}
impl Vertex {
	fn desc() -> wgpu::VertexBufferLayout<'static> {
        wgpu::VertexBufferLayout {
            array_stride: std::mem::size_of::<Vertex>() as wgpu::BufferAddress,
            step_mode: wgpu::VertexStepMode::Vertex,
            attributes: &[
                wgpu::VertexAttribute {
                    offset: 0,
                    shader_location: 0,
                    format: wgpu::VertexFormat::Float32x3,
                },
                wgpu::VertexAttribute {
                    offset: std::mem::size_of::<[f32; 3]>() as wgpu::BufferAddress,
                    shader_location: 1,
                    format: wgpu::VertexFormat::Float32x3,
                }
            ]
        }
    }
}
const TRIANGLE_VERTICES: &[Vertex] = &[
    Vertex { position: [0.0, 0.5, 0.0], color: [1.0, 0.0, 0.0] },
    Vertex { position: [-0.5, -0.5, 0.0], color: [0.0, 1.0, 0.0] },
    Vertex { position: [0.5, -0.5, 0.0], color: [0.0, 0.0, 1.0] },
];

#[repr(C)]
#[derive(Copy, Clone, bytemuck::Pod, bytemuck::Zeroable)]
pub struct RawInstanceData {
    model: [[f32; 4]; 4],
}

impl RawInstanceData {
	pub fn new(model: Matrix4<f32>) -> Self {
		Self {
			model: (model).into(),
		}
	}

	fn desc() -> wgpu::VertexBufferLayout<'static> {
        use std::mem;
        wgpu::VertexBufferLayout {
            array_stride: mem::size_of::<RawInstanceData>() as wgpu::BufferAddress,
            // We need to switch from using a step mode of Vertex to Instance
            // This means that our shaders will only change to use the next
            // instance when the shader starts processing a new instance
            step_mode: wgpu::VertexStepMode::Instance,
            attributes: &[
                // A mat4 takes up 4 vertex slots as it is technically 4 vec4s. We need to define a slot
                // for each vec4. We'll have to reassemble the mat4 in the shader.
                wgpu::VertexAttribute {
                    offset: 0,
                    // While our vertex shader only uses locations 0, and 1 now, in later tutorials, we'll
                    // be using 2, 3, and 4, for Vertex. We'll start at slot 5, not conflict with them later
                    shader_location: 5,
                    format: wgpu::VertexFormat::Float32x4,
                },
                wgpu::VertexAttribute {
                    offset: mem::size_of::<[f32; 4]>() as wgpu::BufferAddress,
                    shader_location: 6,
                    format: wgpu::VertexFormat::Float32x4,
                },
                wgpu::VertexAttribute {
                    offset: mem::size_of::<[f32; 8]>() as wgpu::BufferAddress,
                    shader_location: 7,
                    format: wgpu::VertexFormat::Float32x4,
                },
                wgpu::VertexAttribute {
                    offset: mem::size_of::<[f32; 12]>() as wgpu::BufferAddress,
                    shader_location: 8,
                    format: wgpu::VertexFormat::Float32x4,
                },
            ],
        }
    }
}

pub struct Renderer<'a> {
	surface: wgpu::Surface<'a>,
	device: wgpu::Device,
	queue: wgpu::Queue,
	swapchain_config: wgpu::SurfaceConfiguration,
	render_pipeline: wgpu::RenderPipeline,
	triangle_vertex_buffer: wgpu::Buffer,
	instance_buffer: wgpu::Buffer,

	instances: Vec<Transform>,
}

impl<'a> Renderer<'a> {
	pub async fn new(window: &'a Window) -> anyhow::Result<Self> {
		let mut size = window.inner_size();
		size.width = size.width.max(1);
		size.height = size.height.max(1);

		let instance = wgpu::Instance::default();

		let surface = instance.create_surface(window)?;
		let adapter = instance
			.request_adapter(&wgpu::RequestAdapterOptions {
				power_preference: wgpu::PowerPreference::default(),
				force_fallback_adapter: false,
				// Request an adapter which can render to our surface
				compatible_surface: Some(&surface),
			})
			.await
			.ok_or(anyhow::anyhow!("Failed to find an appropriate adapter"))?;

		// Create the logical device and command queue
		let (device, queue) = adapter
			.request_device(
				&wgpu::DeviceDescriptor {
					label: None,
					required_features: wgpu::Features::empty(),
					// Make sure we use the texture resolution limits from the adapter, so we can support images the size of the swapchain.
					required_limits: wgpu::Limits::downlevel_webgl2_defaults()
						.using_resolution(adapter.limits()),
				},
				None,
			)
			.await?;

		let triangle_vertex_buffer = device.create_buffer_init(
			&wgpu::util::BufferInitDescriptor {
				label: Some("Vertex Buffer"),
				contents: bytemuck::cast_slice(TRIANGLE_VERTICES),
				usage: wgpu::BufferUsages::VERTEX,
			}
		);

		let instances: Vec<Transform> = (0..10).map(|i| Transform::new(Vector3::new(i as f32 * 0.1, 0.0, 0.0))).collect::<_>();
		let raw_instance_data = instances.iter().map(|instance| RawInstanceData::new(instance.model_matrix())).collect::<Vec<_>>();
		let instance_buffer = device.create_buffer_init(
			&wgpu::util::BufferInitDescriptor {
				label: Some("Instance Buffer"),
				contents: bytemuck::cast_slice(&raw_instance_data),
				usage: wgpu::BufferUsages::VERTEX,
			}
		);

		// Load the shaders from disk
		let triangle_shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
			label: None,
			source: wgpu::ShaderSource::Wgsl(Cow::Borrowed(include_str!("../../../assets/shaders/triangle.wgsl"))),
		});

		let triangle_pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
			label: None,
			bind_group_layouts: &[],
			push_constant_ranges: &[],
		});

		let swapchain_capabilities = surface.get_capabilities(&adapter);
		let swapchain_format = swapchain_capabilities.formats[0];

		let render_pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
			label: None,
			layout: Some(&triangle_pipeline_layout),
			vertex: wgpu::VertexState {
				module: &triangle_shader,
				entry_point: "vs_main",
				buffers: &[Vertex::desc(), RawInstanceData::desc()],
			},
			fragment: Some(wgpu::FragmentState {
				module: &triangle_shader,
				entry_point: "fs_main",
				targets: &[Some(swapchain_format.into())],
			}),
			primitive: wgpu::PrimitiveState::default(),
			depth_stencil: None,
			multisample: wgpu::MultisampleState::default(),
			multiview: None,
		});

		let swapchain_config = surface
			.get_default_config(&adapter, size.width, size.height)
			.ok_or(anyhow::anyhow!("Failed to get surface config"))?;
		surface.configure(&device, &swapchain_config);

		Ok(Self {
			surface,
			device,
			queue,
			swapchain_config,
			render_pipeline,
			triangle_vertex_buffer,
			instance_buffer,
			instances,
		})
	}

	pub fn resize(&mut self, new_size: PhysicalSize<u32>) {
		self.swapchain_config.width = new_size.width.max(1);
		self.swapchain_config.height = new_size.height.max(1);
		self.surface.configure(&self.device, &self.swapchain_config);

		#[cfg(target_os = "macos")]
		self.window.request_redraw();
	}

	pub fn update(&mut self) {
		let frame = self.surface
			.get_current_texture()
			.expect("Failed to acquire next swap chain texture");

		let view = frame
			.texture
			.create_view(&wgpu::TextureViewDescriptor::default());

		let mut encoder = self.device.create_command_encoder(
			&wgpu::CommandEncoderDescriptor {
				label: None,
			});

		{
			let mut rpass =
				encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
					label: None,
					color_attachments: &[Some(wgpu::RenderPassColorAttachment {
						view: &view,
						resolve_target: None,
						ops: wgpu::Operations {
							load: wgpu::LoadOp::Clear(wgpu::Color::BLACK),
							store: wgpu::StoreOp::Store,
						},
					})],
					depth_stencil_attachment: None,
					timestamp_writes: None,
					occlusion_query_set: None,
				});
			rpass.set_pipeline(&self.render_pipeline);
			rpass.set_vertex_buffer(0, self.triangle_vertex_buffer.slice(..));
			rpass.set_vertex_buffer(1, self.instance_buffer.slice(..));
			rpass.draw(0..TRIANGLE_VERTICES.len() as u32, 0..self.instances.len() as u32);
		}

		self.queue.submit(Some(encoder.finish()));

		frame.present();
	}
}
