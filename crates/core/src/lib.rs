use camera_controller::CameraController;
use cgmath::{Quaternion, Vector3, Zero};
use sphynx_input::Input;
use sphynx_logging::*;
use sphynx_rendering::{Renderer, Transform};
use std::sync::Arc;
use std::time::Instant;
use winit::{
	application::ApplicationHandler,
	dpi::{PhysicalPosition, PhysicalSize},
	event::{DeviceEvent, MouseButton, WindowEvent},
	event_loop::{ActiveEventLoop, EventLoop},
	keyboard::KeyCode,
	window::{Fullscreen, Window},
};

mod camera_controller;

mod game;
pub use game::GameApi;

#[derive(Debug, Default)]
pub struct EngineConfig {
	pub fullscreen: bool,
	pub vsync: bool,
}
impl EngineConfig {
	pub fn new(fullscreen: bool, vsync: bool) -> Self {
		Self { fullscreen, vsync }
	}

	pub fn run<G, S>(
		self,
		game_api: GameApi<G, S>,
		#[cfg(feature = "hot-reload")] hot_reload_observer: hot_lib_reloader::LibReloadObserver,
	) -> anyhow::Result<()> {
		let engine = Engine::<G, S>::new(
			self,
			game_api,
			#[cfg(feature = "hot-reload")]
			hot_reload_observer,
		)?;
		let event_loop = EventLoop::new()?;
		engine.run(event_loop)
	}
}

struct Engine<G, S> {
	config: EngineConfig,
	window_and_renderer: Option<(Arc<Window>, Renderer)>,
	input: Input,
	last_update_time: Instant,
	frame_counter: u64,
	camera_controller: CameraController,
	restore_cursor_position: PhysicalPosition<f64>, // where should the captures cursor be restore to
	game_api: GameApi<G, S>,
	/// this is a mut ptr since it should support hot-reloading the game code -> memory layout shifts
	game_ptr: *mut G,
	#[cfg(feature = "hot-reload")]
	hot_reload_observer: hot_lib_reloader::LibReloadObserver,
	#[cfg(feature = "egui")]
	inspector_state: sphynx_egui::InspectorState,
	#[cfg(feature = "egui")]
	show_inspector: bool,
}

#[cfg(debug_assertions)]
const BUILD_CONFIG: &str = "Debug";

#[cfg(not(debug_assertions))]
const BUILD_CONFIG: &str = "Release";

impl<G, S> Engine<G, S> {
	fn new(
		config: EngineConfig,
		game_api: GameApi<G, S>,
		#[cfg(feature = "hot-reload")] hot_reload_observer: hot_lib_reloader::LibReloadObserver,
	) -> anyhow::Result<Self> {
		info!(General, "=== INITIALIZING ===");

		let game_ptr = (game_api.create)();

		Ok(Self {
			config,
			window_and_renderer: None,
			input: Input::new(),
			last_update_time: Instant::now(),
			frame_counter: 0,
			camera_controller: CameraController::new(4.0, 3.0),
			restore_cursor_position: PhysicalPosition::new(0.0, 0.0),
			game_ptr,
			game_api,
			#[cfg(feature = "hot-reload")]
			hot_reload_observer,
			#[cfg(feature = "egui")]
			inspector_state: sphynx_egui::InspectorState::default(),
			#[cfg(feature = "egui")]
			show_inspector: false,
		})
	}

	pub fn run(mut self, event_loop: EventLoop<()>) -> anyhow::Result<()> {
		event_loop.run_app(&mut self)?;

		info!(General, "=== SHUTDOWN ===");

		Ok(())
	}

	fn update(&mut self, event_loop: &ActiveEventLoop) {
		let (window, renderer) = self
			.window_and_renderer
			.as_mut()
			.expect("Window and Renderer should have been created by now");

		let now = Instant::now();
		let delta_time = (now - self.last_update_time).as_secs_f32();
		self.last_update_time = now;

		if self.input.was_key_pressed(KeyCode::F11) {
			if self.config.fullscreen {
				window.set_fullscreen(None);
				self.config.fullscreen = false;
			} else {
				window.set_fullscreen(Some(Fullscreen::Borderless(None)));
				self.config.fullscreen = true;
			}
		}

		if self.input.was_key_pressed(KeyCode::Escape) {
			event_loop.exit();
		}

		// captured cursor
		window.set_cursor_visible(!self.input.is_mouse_button_down(MouseButton::Right));
		if self.input.was_mouse_button_pressed(MouseButton::Right) {
			self.restore_cursor_position = self.input.cursor_position;
		} else if self.input.was_mouse_button_released(MouseButton::Right) {
			let _ = window.set_cursor_position(self.restore_cursor_position);
		}

		self.camera_controller
			.update(&self.input, &mut renderer.camera, delta_time);

		renderer.instances.resize(2, Transform::default());
		renderer.instances[0] = Transform::new(
			Vector3::new(0.0, -1.0, 0.0),
			Quaternion::zero(),
			Vector3::new(100.0, 1.0, 100.0),
		);
		renderer.instances[1] = Transform::new(
			Vector3::new(0.0, 0.0, 0.0),
			Quaternion::zero(),
			Vector3::new(0.1, 0.1, 0.1),
		);

		#[cfg(feature = "hot-reload")]
		game::try_reload_game(
			&self.hot_reload_observer,
			&self.game_api,
			&mut self.game_ptr,
		);

		unsafe { (self.game_api.update)(self.game_ptr, delta_time, self.frame_counter) };

		#[cfg(feature = "egui")]
		let (world_ptr, component_types) = unsafe {
			(
				(self.game_api.get_hecs_world)(self.game_ptr),
				(self.game_api.get_component_types)(),
			)
		};

		renderer.update(
			#[cfg(feature = "egui")]
			|ctx| {
				sphynx_egui::debug_window(
					ctx,
					delta_time,
					(self.game_api.version)(),
					&mut self.show_inspector,
				);

				if self.show_inspector && !world_ptr.is_null() {
					sphynx_egui::entity_inspector(
						ctx,
						unsafe { &mut *world_ptr },
						&component_types,
						&mut self.inspector_state,
					);
				}
			},
		);

		self.input.clear_frame();

		self.frame_counter += 1;
	}
}

impl<G, S> ApplicationHandler for Engine<G, S> {
	fn resumed(&mut self, event_loop: &winit::event_loop::ActiveEventLoop) {
		let window = Arc::new(
			event_loop
				.create_window(
					Window::default_attributes()
						.with_title(format!("SphynxEngine [{}]", BUILD_CONFIG))
						.with_inner_size(PhysicalSize::new(1920.0, 1080.0))
						.with_fullscreen(if self.config.fullscreen {
							Some(Fullscreen::Borderless(None))
						} else {
							None
						}),
				)
				.expect("failed to create window"),
		);

		let renderer = pollster::block_on(Renderer::new(window.clone(), self.config.vsync))
			.expect("failed to create renderer");

		self.window_and_renderer = Some((window, renderer));
	}

	fn about_to_wait(&mut self, _event_loop: &ActiveEventLoop) {
		if let Some((window, _renderer)) = self.window_and_renderer.as_ref() {
			window.request_redraw();
		}
	}

	fn window_event(
		&mut self,
		event_loop: &winit::event_loop::ActiveEventLoop,
		_window_id: winit::window::WindowId,
		event: WindowEvent,
	) {
		let (window, renderer) = self
			.window_and_renderer
			.as_mut()
			.expect("Window and Renderer should have been created by now");

		#[cfg(feature = "egui")]
		renderer.egui_renderer.handle_input(window.as_ref(), &event);

		#[cfg(not(feature = "egui"))]
		let _ = window;

		match event {
			WindowEvent::Resized(new_size) => renderer.resize(new_size),
			WindowEvent::RedrawRequested => self.update(event_loop),
			WindowEvent::CloseRequested => event_loop.exit(),

			// INPUT
			WindowEvent::KeyboardInput { event, .. } => self.input.handle_keyboard(event),
			WindowEvent::MouseInput { button, state, .. } => self.input.handle_mouse(button, state),
			WindowEvent::CursorMoved { position, .. } => {
				self.input.handle_cursor_movement(position)
			}
			_ => {}
		}
	}

	fn device_event(
		&mut self,
		_event_loop: &ActiveEventLoop,
		_device_id: winit::event::DeviceId,
		event: DeviceEvent,
	) {
		if let DeviceEvent::MouseMotion { delta } = event {
			self.input.handle_mouse_movement(delta);
		}
	}
}
impl<G, S> Drop for Engine<G, S> {
	fn drop(&mut self) {
		unsafe { (self.game_api.destroy)(self.game_ptr) };
		self.game_ptr = std::ptr::null_mut();
	}
}
