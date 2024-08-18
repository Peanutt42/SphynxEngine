use camera_controller::CameraController;
use sphynx_logging::*;
use sphynx_input::Input;
use sphynx_rendering::{Renderer, Transform};
use winit::{
	dpi::PhysicalSize, event::{Event, WindowEvent}, event_loop::EventLoop, window::{Window, WindowBuilder}
};
use cgmath::{Quaternion, Vector3, Zero};
use std::time::Instant;
use std::sync::Arc;

mod camera_controller;

pub struct EngineConfig {
	pub vsync: bool,
}

impl EngineConfig {
	pub fn new(vsync: bool) -> Self {
		Self {
			vsync,
		}
	}

	pub async fn run(self) -> anyhow::Result<()> {
		let event_loop = EventLoop::new()?;
		let engine = Engine::new(self, &event_loop).await?;
		engine.run(event_loop)
	}
}

impl Default for EngineConfig {
	fn default() -> Self {
		Self::new(false)
	}
}

struct Engine {
	window: Arc<Window>,
	renderer: Renderer,
	input: Input,
	start_time: Instant,
	last_update_time: Instant,
	camera_controller: CameraController,
}

#[cfg(debug_assertions)]
const BUILD_CONFIG: &str = "Debug";

#[cfg(not(debug_assertions))]
const BUILD_CONFIG: &str = "Release";

impl Engine {
	async fn new(config: EngineConfig, event_loop: &EventLoop<()>) -> anyhow::Result<Self> {
		info!(General, "=== INITIALIZING ===");

		let window = Arc::new(
			WindowBuilder::new()
				.with_title(format!("SphynxEngine [{}]", BUILD_CONFIG).as_str())
				.with_inner_size(PhysicalSize::new(1920.0, 1080.0))
				.build(event_loop)?
		);
		let renderer = Renderer::new(window.clone(), config.vsync).await?;

		Ok(Self {
			window,
			renderer,
			input: Input::new(),
			start_time: Instant::now(),
			last_update_time: Instant::now(),
			camera_controller: CameraController::new(4.0, 3.0),
		})
	}

	pub fn run(mut self, event_loop: EventLoop<()>) -> anyhow::Result<()> {
		event_loop.run(move |event, target| {
			self.window.request_redraw();

			match event {
				Event::WindowEvent { event, .. } => {
					match event {
						WindowEvent::Resized(new_size) => self.renderer.resize(new_size),
						WindowEvent::RedrawRequested => self.update(),
						WindowEvent::CloseRequested => target.exit(),

						// INPUT
						WindowEvent::KeyboardInput { event, .. } => self.input.handle_keyboard(event),
						WindowEvent::MouseInput { button, state, .. } => self.input.handle_mouse(button, state),
						_ => {}
					}
				},
				Event::DeviceEvent { event: winit::event::DeviceEvent::MouseMotion { delta, .. }, .. } => self.input.handle_mouse_movement(delta),
				_ => {},
			}
		})?;

		info!(General, "=== SHUTDOWN ===");

		Ok(())
	}

	fn update(&mut self) {
		let now = Instant::now();
		let delta_time = (now - self.last_update_time).as_secs_f32();
		self.last_update_time = now;

		let time = (now - self.start_time).as_secs_f32();

		self.renderer.instances.resize(Renderer::MAX_INSTANCES, Transform::default());
		for (i, instance) in self.renderer.instances.iter_mut().enumerate() {
			let length = (Renderer::MAX_INSTANCES as f32).sqrt();
			*instance = Transform::new(
				Vector3::new(
					2.0 * ((i as f32) % length),
					f32::sin(1.5 * time + 0.25 * (i as f32 % length) + 0.5 * f32::cos(0.5 * time + 0.35 * (i as f32 / length))),
					2.0 * (i as f32 / length)
				),
				Quaternion::zero(),//Quaternion::from_axis_angle(Vector3::new(0.0, 1.0, 0.0), cgmath::Deg(i as f32 * 10.0)),
				Vector3::new(1.0, 1.0, 1.0)//Vector3::new(1.0, 1.5 + 0.5 * f32::sin(time + 0.4 * i as f32), 1.0)
			);
		}

		self.camera_controller.update(&self.input, &mut self.renderer.camera, delta_time);

		self.renderer.update();

		self.input.clear_frame();
	}
}
