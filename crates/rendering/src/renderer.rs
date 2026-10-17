use crate::{
	camera_uniform::CameraUniform,
	depth_texture::DepthTexture,
	include_model, include_shader,
	instance_buffer::InstanceBuffer,
	instance_data::Model_InstanceData,
	lighting::LightUniform,
	load_obj_model,
	shader::{CAMERA_UNIFORM_BIND_GROUP, LIGHT_UNIFORM_BIND_GROUP},
	uniform_buffer::UniformBuffer,
	Camera, Mesh, Shader, Transform,
};
use sphynx_logging::info;
use std::sync::Arc;
use wgpu::{
	Color, CommandEncoderDescriptor, CurrentSurfaceTexture, Device, DeviceDescriptor,
	ExperimentalFeatures, Features, Instance, Limits, LoadOp, MemoryHints, Operations,
	PowerPreference, PresentMode, Queue, RenderPassColorAttachment, RenderPassDescriptor,
	RequestAdapterOptions, StoreOp, Surface, SurfaceConfiguration, TextureViewDescriptor, Trace,
};
use winit::dpi::PhysicalSize;
use winit::window::Window;

pub struct Renderer {
	surface: Surface<'static>,
	window: Arc<Window>,
	device: Device,
	queue: Queue,
	swapchain_config: SurfaceConfiguration,
	depth_texture: DepthTexture,
	default_shader: Shader,
	cube_mesh: Mesh,
	cube_instance_buffer: InstanceBuffer<Model_InstanceData>,

	pub instances: Vec<Transform>,

	light_uniform_buffer: UniformBuffer<LightUniform>,

	pub camera: Camera,
	camera_uniform_buffer: UniformBuffer<CameraUniform>,
}

impl Renderer {
	pub const MAX_INSTANCES: usize = 100;

	pub async fn new(window: Arc<Window>, vsync: bool) -> anyhow::Result<Self> {
		let mut size = window.inner_size();
		size.width = size.width.max(1);
		size.height = size.height.max(1);

		let instance = Instance::default();

		let surface = instance.create_surface(window.clone())?;
		let adapter = instance
			.request_adapter(&RequestAdapterOptions {
				power_preference: PowerPreference::default(),
				force_fallback_adapter: false,
				compatible_surface: Some(&surface),
				apply_limit_buckets: false,
			})
			.await
			.map_err(|e| anyhow::anyhow!("Failed to find an appropriate adapter: {e}"))?;

		let adapter_info = adapter.get_info();
		info!(
			Rendering,
			"{:?}: {}", adapter_info.backend, adapter_info.name
		);

		let (device, queue) = adapter
			.request_device(&DeviceDescriptor {
				label: None,
				required_features: Features::empty(),
				required_limits: Limits::downlevel_webgl2_defaults()
					.using_resolution(adapter.limits()),
				experimental_features: ExperimentalFeatures::default(),
				memory_hints: MemoryHints::default(),
				trace: Trace::default(),
			})
			.await?;

		let swapchain_capabilities = surface.get_capabilities(&adapter);
		let swapchain_format = swapchain_capabilities.formats[0];
		let mut swapchain_config = surface
			.get_default_config(&adapter, size.width, size.height)
			.ok_or(anyhow::anyhow!("Failed to get surface config"))?;
		swapchain_config.present_mode = if vsync {
			PresentMode::AutoVsync
		} else {
			PresentMode::AutoNoVsync
		};
		surface.configure(&device, &swapchain_config);

		let depth_texture = DepthTexture::new(&device, &swapchain_config);

		let cube_mesh = include_model!("../../../assets/models/cube.obj", &device)?; //Mesh::with_vertices(CUBE_VERTICES, Some(CUBE_INDICIES), &device);

		let mut instances: Vec<Transform> = (0..Self::MAX_INSTANCES)
			.map(|_| Transform::default())
			.collect::<_>();
		let raw_instance_data = instances
			.iter()
			.map(|instance| instance.to_model_instance_data())
			.collect::<Vec<_>>();
		let cube_instance_buffer = InstanceBuffer::new(raw_instance_data, &device);
		instances.clear();

		let camera = Camera::default();
		let camera_uniform = CameraUniform::from_camera(
			&camera,
			Camera::get_aspect_from_swapchain(&swapchain_config),
		);
		let camera_uniform_buffer = UniformBuffer::for_vertex_and_fragment(
			camera_uniform,
			Some("CameraUniformBuffer"),
			&device,
		);

		let light_uniform = LightUniform::new([0.0, 0.0, 0.0], [0.5, 0.5, 0.5]);
		let light_uniform_buffer =
			UniformBuffer::for_fragment(light_uniform, Some("LightUniformBuffer"), &device);

		let default_shader = include_shader!(
			"../../../assets/shaders/default.wgsl",
			&[
				Some(camera_uniform_buffer.get_binding()),
				Some(light_uniform_buffer.get_binding())
			],
			&device,
			swapchain_format
		);

		Ok(Self {
			surface,
			window,
			device,
			queue,
			swapchain_config,
			depth_texture,
			default_shader,
			cube_mesh,
			cube_instance_buffer,
			camera,
			camera_uniform_buffer,
			light_uniform_buffer,
			instances,
		})
	}

	pub fn resize(&mut self, new_size: PhysicalSize<u32>) {
		self.reconfigure(new_size);
	}

	/// (Re)configures the swapchain and recreates the size-dependent
	/// attachments for `new_size`. Safe to call whenever the surface may have
	/// been invalidated (resizes, tiling window managers, fullscreening, ...).
	fn reconfigure(&mut self, new_size: PhysicalSize<u32>) {
		self.swapchain_config.width = new_size.width.max(1);
		self.swapchain_config.height = new_size.height.max(1);
		self.surface.configure(&self.device, &self.swapchain_config);

		self.depth_texture = DepthTexture::new(&self.device, &self.swapchain_config);
	}

	pub fn update(&mut self) {
		let frame = match self.surface.get_current_texture() {
			CurrentSurfaceTexture::Success(frame) => frame,
			// outdated surface texture -> recreate
			CurrentSurfaceTexture::Lost
			| CurrentSurfaceTexture::Outdated
			| CurrentSurfaceTexture::Suboptimal(_) => {
				let size = self.window.inner_size();
				self.reconfigure(size);
				return;
			}
			CurrentSurfaceTexture::Timeout | CurrentSurfaceTexture::Occluded => return,
			CurrentSurfaceTexture::Validation => {
				panic!("Validation error on current surface texture");
			}
		};

		let view = frame.texture.create_view(&TextureViewDescriptor::default());

		self.cube_instance_buffer.instances = self
			.instances
			.iter()
			.map(|instance| instance.to_model_instance_data())
			.collect::<Vec<_>>();
		self.cube_instance_buffer.update(&self.queue);

		self.camera_uniform_buffer.update(
			CameraUniform::from_camera(
				&self.camera,
				Camera::get_aspect_from_swapchain(&self.swapchain_config),
			),
			&self.queue,
		);

		let mut encoder = self
			.device
			.create_command_encoder(&CommandEncoderDescriptor { label: None });

		encoder.push_debug_group("3d scene renderer");
		{
			let mut rpass = encoder.begin_render_pass(&RenderPassDescriptor {
				label: Some("Main Renderpass"),
				color_attachments: &[Some(RenderPassColorAttachment {
					view: &view,
					resolve_target: None,
					ops: Operations {
						load: LoadOp::Clear(Color::BLACK),
						store: StoreOp::Store,
					},
					depth_slice: None,
				})],
				depth_stencil_attachment: Some(self.depth_texture.get_attachment()),
				timestamp_writes: None,
				occlusion_query_set: None,
				multiview_mask: None,
			});

			self.camera_uniform_buffer
				.bind(CAMERA_UNIFORM_BIND_GROUP, &mut rpass);
			self.light_uniform_buffer
				.bind(LIGHT_UNIFORM_BIND_GROUP, &mut rpass);
			self.default_shader.bind(&mut rpass);
			self.cube_mesh
				.draw_instanced(&self.cube_instance_buffer, &mut rpass);
		}
		encoder.pop_debug_group();

		self.queue.submit(Some(encoder.finish()));
		self.queue.present(frame);
	}
}
