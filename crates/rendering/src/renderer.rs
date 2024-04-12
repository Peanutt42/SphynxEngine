use winit::dpi::PhysicalSize;
use winit::window::Window;
use wgpu::util::DeviceExt;
use cgmath::Vector3;
use std::time::Instant;
use std::sync::Arc;
use crate::{
	vertex::PC_Vertex,
	instance::Model_InstanceData,
	shader::{VERTEX_BUFFER_BIND_SLOT, INSTANCE_BUFFER_BIND_SLOT},
	Transform,
	Mesh,
	Shader
};

const TRIANGLE_VERTICES: &[PC_Vertex] = &[
    PC_Vertex { position: [0.0, 0.5, 0.0], color: [1.0, 0.0, 0.0] },
    PC_Vertex { position: [-0.5, -0.5, 0.0], color: [0.0, 1.0, 0.0] },
    PC_Vertex { position: [0.5, -0.5, 0.0], color: [0.0, 0.0, 1.0] },
];

pub struct Renderer {
	surface: wgpu::Surface<'static>,
	device: wgpu::Device,
	queue: wgpu::Queue,
	swapchain_config: wgpu::SurfaceConfiguration,
	triangle_shader: Shader,
	triangle_mesh: Mesh,
	triangle_instance_buffer: wgpu::Buffer,

	instances: Vec<Transform>,
	start_time: Instant,
}

impl Renderer {
	pub async fn new(window: Arc<Window>) -> anyhow::Result<Self> {
		let mut size = window.inner_size();
		size.width = size.width.max(1);
		size.height = size.height.max(1);

		let instance = wgpu::Instance::default();

		let surface = instance.create_surface(window)?;
		let adapter = instance
			.request_adapter(&wgpu::RequestAdapterOptions {
				power_preference: wgpu::PowerPreference::default(),
				force_fallback_adapter: false,
				compatible_surface: Some(&surface),
			})
			.await
			.ok_or(anyhow::anyhow!("Failed to find an appropriate adapter"))?;

		let (device, queue) = adapter
			.request_device(
				&wgpu::DeviceDescriptor {
					label: None,
					required_features: wgpu::Features::empty(),
					required_limits: wgpu::Limits::downlevel_webgl2_defaults()
						.using_resolution(adapter.limits()),
				},
				None,
			)
			.await?;

		let triangle_mesh = Mesh::with_vertices(TRIANGLE_VERTICES, &device);

		let instances: Vec<Transform> = (0..10)
			.map(|i|
				Transform::new(Vector3::new(i as f32 * 0.1, 0.0, 0.0))
			)
			.collect::<_>();
		let raw_instance_data = instances.iter().map(|instance| Model_InstanceData::new(instance.model_matrix())).collect::<Vec<_>>();
		let triangle_instance_buffer = device.create_buffer_init(
			&wgpu::util::BufferInitDescriptor {
				label: Some("Instance Buffer"),
				contents: bytemuck::cast_slice(&raw_instance_data),
				usage: wgpu::BufferUsages::VERTEX | wgpu::BufferUsages::COPY_DST,
			}
		);

		let swapchain_capabilities = surface.get_capabilities(&adapter);
		let swapchain_format = swapchain_capabilities.formats[0];
		let swapchain_config = surface
			.get_default_config(&adapter, size.width, size.height)
			.ok_or(anyhow::anyhow!("Failed to get surface config"))?;
		surface.configure(&device, &swapchain_config);

		let triangle_shader = Shader::from_str_instanced::<PC_Vertex, Model_InstanceData>(include_str!("../../../assets/shaders/triangle.wgsl"), Some("Triangle Shader"), &device, swapchain_format);

		Ok(Self {
			surface,
			device,
			queue,
			swapchain_config,
			triangle_shader,
			triangle_mesh,
			triangle_instance_buffer,
			instances,
			start_time: Instant::now(),
		})
	}

	pub fn resize(&mut self, new_size: PhysicalSize<u32>) {
		self.swapchain_config.width = new_size.width.max(1);
		self.swapchain_config.height = new_size.height.max(1);
		self.surface.configure(&self.device, &self.swapchain_config);

		// only on macos you need to explicitly redraw the window
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

		let time = (Instant::now() - self.start_time).as_secs_f32();
		self.instances = (0..10)
			.map(|i|
				Transform::new(Vector3::new(i as f32 * 0.1, 0.5 * f32::sin(time + 0.2 * i as f32), 0.0))
			)
			.collect::<_>();
		let raw_instance_data = self.instances.iter().map(|instance| Model_InstanceData::new(instance.model_matrix())).collect::<Vec<_>>();
		
		self.queue.write_buffer(&self.triangle_instance_buffer, 0, bytemuck::cast_slice(&raw_instance_data));
		
		let mut encoder = self.device.create_command_encoder(
			&wgpu::CommandEncoderDescriptor {
				label: None,
			});

		{
			let mut rpass =
				encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
					label: Some("Main Renderpass"),
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
			rpass.set_pipeline(self.triangle_shader.get_pipeline());
			rpass.set_vertex_buffer(VERTEX_BUFFER_BIND_SLOT, self.triangle_mesh.get_vertex_buffer_slice());
			rpass.set_vertex_buffer(INSTANCE_BUFFER_BIND_SLOT, self.triangle_instance_buffer.slice(..));
			rpass.draw(0..TRIANGLE_VERTICES.len() as u32, 0..self.instances.len() as u32);
		}

		self.queue.submit(Some(encoder.finish()));

		frame.present();
	}
}
