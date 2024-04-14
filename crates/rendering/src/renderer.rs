use winit::dpi::PhysicalSize;
use winit::window::Window;
use cgmath::Vector3;
use std::sync::Arc;
use sphynx_logging::info;
use crate::{
	Camera, Mesh, Shader, Transform,
	include_shader,
	vertex::PC_Vertex,
	depth_texture::DepthTexture,
	instance_buffer::InstanceBuffer,
	camera_uniform::{CameraUniform, CameraUniformBuffer},
	instance_data::Model_InstanceData,
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
	depth_texture: DepthTexture,
	triangle_shader: Shader,
	triangle_mesh: Mesh,
	triangle_instance_buffer: InstanceBuffer<Model_InstanceData>,

	pub instances: Vec<Transform>,
	pub camera: Camera,
	camera_uniform_buffer: CameraUniformBuffer,
}

impl Renderer {
	pub async fn new(window: Arc<Window>, vsync: bool) -> anyhow::Result<Self> {
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

		let adapter_info = adapter.get_info();
		info!(Rendering, "{:?}: {}", adapter_info.backend, adapter_info.name);

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
		let mut swapchain_config = surface
			.get_default_config(&adapter, size.width, size.height)
			.ok_or(anyhow::anyhow!("Failed to get surface config"))?;
		swapchain_config.present_mode = if vsync { wgpu::PresentMode::AutoVsync } else { wgpu::PresentMode::AutoNoVsync };
		surface.configure(&device, &swapchain_config);

		let depth_texture = DepthTexture::new(&device, &swapchain_config);

		let camera = Camera::default();
		let camera_uniform = CameraUniform::new(camera.get_projection_view_matrix(Camera::get_aspect(swapchain_config.width as f32, swapchain_config.height as f32)));
		let camera_uniform_buffer = CameraUniformBuffer::new(camera_uniform, &device);

		let triangle_shader = include_shader!(
			"../../../assets/shaders/triangle.wgsl",
			&[camera_uniform_buffer.get_binding()],
			&device,
			swapchain_format
		);

		Ok(Self {
			surface,
			device,
			queue,
			swapchain_config,
			depth_texture,
			triangle_shader,
			triangle_mesh,
			triangle_instance_buffer,
			camera,
			camera_uniform_buffer,
			instances,
		})
	}

	pub fn resize(&mut self, new_size: PhysicalSize<u32>) {
		self.swapchain_config.width = new_size.width.max(1);
		self.swapchain_config.height = new_size.height.max(1);
		self.surface.configure(&self.device, &self.swapchain_config);

		self.depth_texture = DepthTexture::new(&self.device, &self.swapchain_config);

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

		let camera_projection_view = self.camera.get_projection_view_matrix(Camera::get_aspect_from_swapchain(&self.swapchain_config));
		self.camera_uniform_buffer.update(CameraUniform::new(camera_projection_view), &self.queue);

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
					depth_stencil_attachment: Some(self.depth_texture.get_attachment()),
					timestamp_writes: None,
					occlusion_query_set: None,
				});

			self.camera_uniform_buffer.bind(&mut rpass);
			self.triangle_shader.bind(&mut rpass);
			self.triangle_mesh.draw_instanced(&self.triangle_instance_buffer, &mut rpass);
		}

		self.queue.submit(Some(encoder.finish()));

		frame.present();
	}
}
