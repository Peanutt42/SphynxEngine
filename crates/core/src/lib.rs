use camera_controller::CameraController;
use sphynx_logging::*;
use sphynx_input::Input;
use sphynx_rendering::{Renderer, Transform};
use winit::{
	dpi::{PhysicalPosition, PhysicalSize}, event::{DeviceEvent, Event, MouseButton, WindowEvent}, event_loop::EventLoop, keyboard::KeyCode, window::{Fullscreen, Window, WindowBuilder}
};
use cgmath::{Quaternion, Vector3, Zero};
use std::time::Instant;
use std::sync::Arc;

mod camera_controller;

pub struct EngineConfig {
	pub fullscreen: bool,
	pub vsync: bool,
}

impl EngineConfig {
	pub fn new(fullscreen: bool, vsync: bool) -> Self {
		Self {
			fullscreen,
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
		Self::new(false, false)
	}
}

struct Engine {
	config: EngineConfig,
	window: Arc<Window>,
	renderer: Renderer,
	input: Input,
	last_update_time: Instant,
	camera_controller: CameraController,
	restore_cursor_position: PhysicalPosition<f64>, // where should the captures cursor be restore to
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
				.with_fullscreen(if config.fullscreen {
					Some(Fullscreen::Borderless(None))
				}
				else {
					None
				})
				.build(event_loop)?
		);
		let renderer = Renderer::new(window.clone(), config.vsync).await?;

		Ok(Self {
			config,
			window,
			renderer,
			input: Input::new(),
			last_update_time: Instant::now(),
			camera_controller: CameraController::new(4.0, 3.0),
			restore_cursor_position: PhysicalPosition::new(0.0, 0.0),
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
						WindowEvent::CursorMoved { position, .. } => self.input.handle_cursor_movement(position),
						_ => {}
					}
				},
				Event::DeviceEvent { event: DeviceEvent::MouseMotion { delta }, .. } => self.input.handle_mouse_movement(delta),
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

		if self.input.was_key_pressed(KeyCode::F11) {
			if self.config.fullscreen {
				self.window.set_fullscreen(None);
				self.config.fullscreen = false;
			}
			else {
				self.window.set_fullscreen(Some(Fullscreen::Borderless(None)));
				self.config.fullscreen = true;
			}
		}

		// captured cursor
		self.window.set_cursor_visible(!self.input.is_mouse_button_down(MouseButton::Right));
		if self.input.was_mouse_button_pressed(MouseButton::Right) {
			self.restore_cursor_position = self.input.cursor_position;
		}
		else if self.input.was_mouse_button_released(MouseButton::Right) {
			let _ = self.window.set_cursor_position(self.restore_cursor_position);
		}

		self.camera_controller.update(&self.input, &mut self.renderer.camera, delta_time);

		self.renderer.instances.resize(2, Transform::default());
		self.renderer.instances[0] = Transform::new(Vector3::new(0.0, -1.0, 0.0), Quaternion::zero(), Vector3::new(100.0, 1.0, 100.0));
		self.renderer.instances[1] = Transform::new(Vector3::new(0.0, 0.0, 0.0), Quaternion::zero(), Vector3::new(0.1, 0.1, 0.1));

		self.renderer.update();

		self.input.clear_frame();
	}
}