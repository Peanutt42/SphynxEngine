use winit::dpi::PhysicalSize;
use winit::window::Window;
use wgpu::{Color, CommandEncoderDescriptor, Device, DeviceDescriptor, Features, Instance, Limits, LoadOp, Operations, PowerPreference, PresentMode, Queue, RenderPassColorAttachment, RenderPassDescriptor, RequestAdapterOptions, StoreOp, Surface, SurfaceConfiguration, TextureViewDescriptor};
use std::sync::Arc;
use sphynx_logging::info;
use crate::{
	camera_uniform::CameraUniform, depth_texture::DepthTexture, include_shader, instance_buffer::InstanceBuffer, instance_data::Model_InstanceData, lighting::LightUniform, shader::{CAMERA_UNIFORM_BIND_GROUP, LIGHT_UNIFORM_BIND_GROUP}, uniform_buffer::UniformBuffer, vertex::PCN_Vertex, Camera, Mesh, Shader, Transform
};

pub struct Renderer {
	surface: Surface<'static>,
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

		let surface = instance.create_surface(window)?;
		let adapter = instance
			.request_adapter(&RequestAdapterOptions {
				power_preference: PowerPreference::default(),
				force_fallback_adapter: false,
				compatible_surface: Some(&surface),
			})
			.await
			.ok_or(anyhow::anyhow!("Failed to find an appropriate adapter"))?;

		let adapter_info = adapter.get_info();
		info!(Rendering, "{:?}: {}", adapter_info.backend, adapter_info.name);

		let (device, queue) = adapter
			.request_device(
				&DeviceDescriptor {
					label: None,
					required_features: Features::empty(),
					required_limits: Limits::downlevel_webgl2_defaults()
						.using_resolution(adapter.limits()),
				},
				None,
			)
			.await?;

		let swapchain_capabilities = surface.get_capabilities(&adapter);
		let swapchain_format = swapchain_capabilities.formats[0];
		let mut swapchain_config = surface
			.get_default_config(&adapter, size.width, size.height)
			.ok_or(anyhow::anyhow!("Failed to get surface config"))?;
		swapchain_config.present_mode =
			if vsync {
				PresentMode::AutoVsync
			}
			else {
				PresentMode::AutoNoVsync
			};
		surface.configure(&device, &swapchain_config);

		let depth_texture = DepthTexture::new(&device, &swapchain_config);

		let cube_mesh = Mesh::with_vertices(CUBE_VERTICES, &device);

		let mut instances: Vec<Transform> = (0..Self::MAX_INSTANCES)
			.map(|_| Transform::default())
			.collect::<_>();
		let raw_instance_data = instances.iter().map(|instance| instance.to_model_instance_data()).collect::<Vec<_>>();
		let cube_instance_buffer = InstanceBuffer::new(raw_instance_data, &device);
		instances.truncate(0);

		let camera = Camera::default();
		let camera_uniform = CameraUniform::from_camera(&camera, Camera::get_aspect_from_swapchain(&swapchain_config));
		let camera_uniform_buffer = UniformBuffer::for_vertex_and_fragment(camera_uniform, Some("CameraUniformBuffer"), &device);

		let light_uniform = LightUniform::new([0.0, 0.0, 0.0], [0.5, 0.5, 0.5]);
		let light_uniform_buffer = UniformBuffer::for_fragment(light_uniform, Some("LightUniformBuffer"), &device);

		let default_shader = include_shader!(
			"../../../assets/shaders/default.wgsl",
			&[camera_uniform_buffer.get_binding(), light_uniform_buffer.get_binding()],
			&device,
			swapchain_format
		);

		Ok(Self {
			surface,
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
			.create_view(&TextureViewDescriptor::default());

		self.cube_instance_buffer.instances = self.instances
			.iter()
			.map(|instance| instance.to_model_instance_data())
			.collect::<Vec<_>>();
		self.cube_instance_buffer.update(&self.queue);

		self.camera_uniform_buffer.update(
			CameraUniform::from_camera(&self.camera, Camera::get_aspect_from_swapchain(&self.swapchain_config)),
			&self.queue
		);

		let mut encoder = self.device.create_command_encoder(
			&CommandEncoderDescriptor {
				label: None,
			});

		{
			let mut rpass =
				encoder.begin_render_pass(&RenderPassDescriptor {
					label: Some("Main Renderpass"),
					color_attachments: &[Some(RenderPassColorAttachment {
						view: &view,
						resolve_target: None,
						ops: Operations {
							load: LoadOp::Clear(Color::BLACK),
							store: StoreOp::Store,
						},
					})],
					depth_stencil_attachment: Some(self.depth_texture.get_attachment()),
					timestamp_writes: None,
					occlusion_query_set: None,
				});

			self.camera_uniform_buffer.bind(CAMERA_UNIFORM_BIND_GROUP, &mut rpass);
			self.light_uniform_buffer.bind(LIGHT_UNIFORM_BIND_GROUP, &mut rpass);
			self.default_shader.bind(&mut rpass);
			self.cube_mesh.draw_instanced(&self.cube_instance_buffer, &mut rpass);
		}

		self.queue.submit(Some(encoder.finish()));

		frame.present();
	}
}

const CUBE_VERTICES: &[PCN_Vertex] = &[
	// Front face
	// Triangle 1
	PCN_Vertex { position: [-0.5, -0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0,  1.0] },
	PCN_Vertex { position: [ 0.5, -0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0,  1.0] },
	PCN_Vertex { position: [ 0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0,  1.0] },
	// Triangle 2
	PCN_Vertex { position: [ 0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0,  1.0] },
	PCN_Vertex { position: [-0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0,  1.0] },
	PCN_Vertex { position: [-0.5, -0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0,  1.0] },

	// Back face
	// Triangle 1
	PCN_Vertex { position: [-0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0, -1.0] },
	PCN_Vertex { position: [ 0.5,  0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0, -1.0] },
	PCN_Vertex { position: [ 0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0, -1.0] },
	// Triangle 2
	PCN_Vertex { position: [ 0.5,  0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0, -1.0] },
	PCN_Vertex { position: [-0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0, -1.0] },
	PCN_Vertex { position: [-0.5,  0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 0.0, -1.0] },

	// Top face
	// Triangle 1
	PCN_Vertex { position: [-0.5,  0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 1.0, 0.0] },
	PCN_Vertex { position: [ 0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 1.0, 0.0] },
	PCN_Vertex { position: [ 0.5,  0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 1.0, 0.0] },
	// Triangle 2
	PCN_Vertex { position: [ 0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 1.0, 0.0] },
	PCN_Vertex { position: [-0.5,  0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 1.0, 0.0] },
	PCN_Vertex { position: [-0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, 1.0, 0.0] },

	// Bottom face
	// Triangle 1
	PCN_Vertex { position: [-0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, -1.0, 0.0] },
	PCN_Vertex { position: [ 0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, -1.0, 0.0] },
	PCN_Vertex { position: [ 0.5, -0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, -1.0, 0.0] },
	// Triangle 2
	PCN_Vertex { position: [ 0.5, -0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, -1.0, 0.0] },
	PCN_Vertex { position: [-0.5, -0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [0.0, -1.0, 0.0] },
	PCN_Vertex { position: [-0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [0.0, -1.0, 0.0] },

	// Left face
	// Triangle 1
	PCN_Vertex { position: [-0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [-1.0, 0.0, 0.0] },
	PCN_Vertex { position: [-0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [-1.0, 0.0, 0.0] },
	PCN_Vertex { position: [-0.5,  0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [-1.0, 0.0, 0.0] },
	// Triangle 2
	PCN_Vertex { position: [-0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [-1.0, 0.0, 0.0] },
	PCN_Vertex { position: [-0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [-1.0, 0.0, 0.0] },
	PCN_Vertex { position: [-0.5, -0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [-1.0, 0.0, 0.0] },

	// Right face
	// Triangle 1
	PCN_Vertex { position: [ 0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [1.0, 0.0, 0.0] },
	PCN_Vertex { position: [ 0.5,  0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [1.0, 0.0, 0.0] },
	PCN_Vertex { position: [ 0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [1.0, 0.0, 0.0] },
	// Triangle 2
	PCN_Vertex { position: [ 0.5,  0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [1.0, 0.0, 0.0] },
	PCN_Vertex { position: [ 0.5, -0.5,  0.5], color: [1.0, 1.0, 1.0], normal: [1.0, 0.0, 0.0] },
	PCN_Vertex { position: [ 0.5, -0.5, -0.5], color: [1.0, 1.0, 1.0], normal: [1.0, 0.0, 0.0] },
];