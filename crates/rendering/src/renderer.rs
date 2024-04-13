use winit::dpi::PhysicalSize;
use winit::window::Window;
use cgmath::Vector3;
use std::sync::Arc;
use crate::{
	include_shader,
	instance_data::{InstanceData, Model_InstanceData},
	instance_buffer::InstanceBuffer,
	shader::{INSTANCE_BUFFER_BIND_SLOT, VERTEX_BUFFER_BIND_SLOT},
	vertex::PC_Vertex,
	Mesh,
	Shader,
	Texture,
	Transform
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
	depth_texture: Texture,
	triangle_shader: Shader,
	triangle_mesh: Mesh,
	triangle_instance_buffer: InstanceBuffer<Model_InstanceData>,

	pub instances: Vec<Transform>,
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
		let triangle_instance_buffer = InstanceBuffer::new(raw_instance_data, &device);

		let swapchain_capabilities = surface.get_capabilities(&adapter);
		let swapchain_format = swapchain_capabilities.formats[0];
		let swapchain_config = surface
			.get_default_config(&adapter, size.width, size.height)
			.ok_or(anyhow::anyhow!("Failed to get surface config"))?;
		surface.configure(&device, &swapchain_config);

		let depth_texture = Texture::create_depth_texture(&device, &swapchain_config);

		let triangle_shader = include_shader!("../../../assets/shaders/triangle.wgsl", &device, swapchain_format);

		Ok(Self {
			surface,
			device,
			queue,
			swapchain_config,
			depth_texture,
			triangle_shader,
			triangle_mesh,
			triangle_instance_buffer,
			instances,
		})
	}

	pub fn resize(&mut self, new_size: PhysicalSize<u32>) {
		self.swapchain_config.width = new_size.width.max(1);
		self.swapchain_config.height = new_size.height.max(1);
		self.surface.configure(&self.device, &self.swapchain_config);

		self.depth_texture = Texture::create_depth_texture(&self.device, &self.swapchain_config);

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

		self.triangle_instance_buffer.instances = self.instances.iter().map(|instance| Model_InstanceData::new(instance.model_matrix())).collect::<Vec<_>>();
		self.triangle_instance_buffer.update(&self.queue);

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
					depth_stencil_attachment: Some(wgpu::RenderPassDepthStencilAttachment {
						view: &self.depth_texture.view,
						depth_ops: Some(wgpu::Operations {
							load: wgpu::LoadOp::Clear(1.0),
							store: wgpu::StoreOp::Store,
						}),
						stencil_ops: None,
					}),
					timestamp_writes: None,
					occlusion_query_set: None,
				});
			self.draw(&mut rpass, &self.triangle_shader, &self.triangle_mesh, Some(&self.triangle_instance_buffer));
		}

		self.queue.submit(Some(encoder.finish()));

		frame.present();
	}

	fn draw<'a, I: InstanceData>(&self, renderpass: &mut wgpu::RenderPass<'a>, shader: &'a Shader, mesh: &'a Mesh, instance_buffer: Option<&'a InstanceBuffer<I>>) {
		renderpass.set_pipeline(&shader.pipeline);
		renderpass.set_vertex_buffer(VERTEX_BUFFER_BIND_SLOT, mesh.vertex_buffer.slice(..));
		if let Some(instance_buffer) = instance_buffer {
			renderpass.set_vertex_buffer(INSTANCE_BUFFER_BIND_SLOT, instance_buffer.buffer.slice(..));
		}
		let instance_count = instance_buffer
			.map(|instance_buffer| instance_buffer.instances.len())
			.unwrap_or(1);
		renderpass.draw(0..self.triangle_mesh.vertex_count as u32, 0..instance_count as u32);
	}
}
