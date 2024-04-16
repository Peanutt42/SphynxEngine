use camera_controller::CameraController;
use sphynx_logging::*;
use sphynx_rendering::{Renderer, Transform};
use winit::{
	dpi::PhysicalSize, event::{Event, KeyEvent, WindowEvent}, event_loop::EventLoop, keyboard::PhysicalKey, event::MouseButton, window::{Window, WindowBuilder}
};
use cgmath::{Quaternion, Rotation3, Vector3};
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

	pub fn run(self) -> anyhow::Result<()> {
		let event_loop = EventLoop::new()?;
		let engine = Engine::new(self, &event_loop)?;
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
	start_time: Instant,
	last_update_time: Instant,
	camera_controller: CameraController,
	right_mouse_pressed: bool,
}

#[cfg(debug_assertions)]
const BUILD_CONFIG: &str = "Debug";

#[cfg(not(debug_assertions))]
const BUILD_CONFIG: &str = "Release";

impl Engine {
	fn new(config: EngineConfig, event_loop: &EventLoop<()>) -> anyhow::Result<Self> {
		info!(General, "=== INITIALIZING ===");

		let window = Arc::new(
			WindowBuilder::new()
				.with_title(format!("SphynxEngine [{}]", BUILD_CONFIG).as_str())
				.with_inner_size(PhysicalSize::new(1920.0, 1080.0))
				.build(event_loop)?
		);
		let renderer = pollster::block_on(Renderer::new(window.clone(), config.vsync))?;

		Ok(Self {
			window,
			renderer,
			start_time: Instant::now(),
			last_update_time: Instant::now(),
			camera_controller: CameraController::new(4.0, 4.0),
			right_mouse_pressed: false,
		})
	}

	pub fn run(mut self, event_loop: EventLoop<()>) -> anyhow::Result<()> {
		sphynx_logging::init();

		event_loop.run(move |event, target| {
			self.window.request_redraw();

			match event {
				Event::WindowEvent { event, .. } => {
					match event {
						WindowEvent::Resized(new_size) => self.renderer.resize(new_size),
						WindowEvent::RedrawRequested => self.update(),
						WindowEvent::CloseRequested => target.exit(),

						// INPUT
						WindowEvent::KeyboardInput {
							event:
								KeyEvent {
									physical_key: PhysicalKey::Code(key_code),
									state,
									..
								},
							..
					 	} => self.camera_controller.process_keyboard(key_code, state),
						WindowEvent::MouseInput { button: MouseButton::Right, state, .. } => self.right_mouse_pressed = state.is_pressed(),
						_ => {}
					}
				},
				Event::DeviceEvent { event, .. } => {
					match event {
						winit::event::DeviceEvent::MouseMotion { delta, .. } => {
							if self.right_mouse_pressed {
								self.camera_controller.process_mouse(delta.0, delta.1);
							}
						},
						_ => {},
					}
				},
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

		self.renderer.instances.resize(45, Transform::default());
		for (i, instance) in self.renderer.instances.iter_mut().enumerate() {
			*instance = Transform::new(
				Vector3::new(i as f32 * 0.4, f32::sin(time + 0.4 * i as f32), f32::cos(time + 0.4 * i as f32)),
				Quaternion::from_axis_angle(Vector3::new(0.0, 1.0, 0.0), cgmath::Deg(i as f32 * 10.0)),
				Vector3::new(1.0, 1.5 + 0.5 * f32::sin(time + 0.4 * i as f32), 1.0)
			);
		}

		self.camera_controller.update_camera(&mut self.renderer.camera, delta_time);

		self.renderer.update();
	}
}
