use camera_controller::CameraController;
use cgmath::{Quaternion, Vector3, Zero};
use sphynx_input::Input;
use sphynx_logging::*;
use sphynx_rendering::{Renderer, Transform};
use std::sync::Arc;
use web_time::Instant;
use winit::{
	application::ApplicationHandler,
	dpi::{PhysicalPosition, PhysicalSize},
	event::{DeviceEvent, MouseButton, WindowEvent},
	event_loop::{ActiveEventLoop, EventLoop},
	keyboard::KeyCode,
	window::{CursorGrabMode, Fullscreen, Window, WindowAttributes},
};

mod camera_controller;

mod game;
pub use game::GameApi;

mod platform;

#[derive(Debug, Default)]
pub struct EngineConfig {
	pub fullscreen: bool,
	pub vsync: bool,
}
impl EngineConfig {
	pub fn new(fullscreen: bool, vsync: bool) -> Self {
		Self { fullscreen, vsync }
	}

	pub fn run<G: 'static, S: 'static>(
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

pub(crate) struct Engine<G, S> {
	config: EngineConfig,
	/// None while the window has not yet been created
	window: Option<Arc<Window>>,
	/// None until the renderer has finished initializing
	renderer: Option<Renderer>,
	/// Renderer creation can be asynchronous (web); the event loop collects it
	/// here and moves it into `renderer` once ready.
	renderer_init: Option<platform::RendererInit>,
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

impl<G: 'static, S: 'static> Engine<G, S> {
	fn new(
		config: EngineConfig,
		game_api: GameApi<G, S>,
		#[cfg(feature = "hot-reload")] hot_reload_observer: hot_lib_reloader::LibReloadObserver,
	) -> anyhow::Result<Self> {
		info!(General, "=== INITIALIZING ===");

		let game_ptr = (game_api.create)();

		Ok(Self {
			config,
			window: None,
			renderer: None,
			renderer_init: None,
			input: Input::new(),
			last_update_time: Instant::now(),
			frame_counter: 0,
			camera_controller: CameraController::new(4.0, 0.005),
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

	pub fn run(self, event_loop: EventLoop<()>) -> anyhow::Result<()> {
		platform::run(self, event_loop)
	}

	fn poll_renderer_init(&mut self) {
		let Some(renderer_init) = self.renderer_init.as_mut() else {
			return;
		};

		let Some(result) = renderer_init.poll() else {
			return;
		};

		self.renderer_init = None;

		match result {
			Ok(renderer) => {
				info!(Rendering, "=== RENDERER READY ===");
				self.last_update_time = Instant::now();
				self.renderer = Some(renderer);
			}
			Err(error) => {
				error!(Rendering, "failed to initialize the renderer: {error:#}");
				std::process::exit(1);
			}
		}
	}

	fn window_attributes(&self) -> WindowAttributes {
		platform::configure_window(
			Window::default_attributes()
				.with_title(format!("SphynxEngine [{}]", BUILD_CONFIG))
				.with_inner_size(PhysicalSize::new(1920.0, 1080.0)),
			self.config.fullscreen,
		)
	}

	fn update(&mut self, event_loop: &ActiveEventLoop) {
		let window = self
			.window
			.as_mut()
			.expect("Window should have been created by now");
		let renderer = self
			.renderer
			.as_mut()
			.expect("Renderer should have been created by now");

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

		#[cfg(target_arch = "wasm32")]
		let _ = event_loop;

		#[cfg(not(target_arch = "wasm32"))]
		if self.input.was_key_pressed(KeyCode::Escape) {
			event_loop.exit();
		}

		// captured cursor
		if self.input.was_mouse_button_pressed(MouseButton::Right) {
			self.restore_cursor_position = self.input.cursor_position;
			let _ = window.set_cursor_grab(CursorGrabMode::Locked);
		} else if self.input.was_mouse_button_released(MouseButton::Right) {
			let _ = window.set_cursor_grab(CursorGrabMode::None);
			let _ = window.set_cursor_position(self.restore_cursor_position);
		}
		window.set_cursor_visible(!self.input.is_mouse_button_down(MouseButton::Right));

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

impl<G: 'static, S: 'static> ApplicationHandler for Engine<G, S> {
	fn resumed(&mut self, event_loop: &winit::event_loop::ActiveEventLoop) {
		let window = Arc::new(
			event_loop
				.create_window(self.window_attributes())
				.expect("failed to create window"),
		);

		self.window = Some(Arc::clone(&window));
		self.renderer_init = Some(platform::RendererInit::start(window, self.config.vsync));

		// on native, this runs blocking, on web, this starts a async task
		self.poll_renderer_init();
	}

	fn about_to_wait(&mut self, _event_loop: &ActiveEventLoop) {
		self.poll_renderer_init();

		if let Some(window) = self.window.as_ref() {
			window.request_redraw();
		}
	}

	fn window_event(
		&mut self,
		event_loop: &winit::event_loop::ActiveEventLoop,
		_window_id: winit::window::WindowId,
		event: WindowEvent,
	) {
		#[allow(unused)]
		let Some(window) = self.window.as_ref() else {
			return;
		};
		let Some(renderer) = self.renderer.as_mut() else {
			return;
		};

		#[cfg(feature = "egui")]
		renderer.egui_renderer.handle_input(window.as_ref(), &event);

		match event {
			WindowEvent::Resized(new_size) => {
				renderer.resize(new_size);
			}
			WindowEvent::RedrawRequested => {
				self.update(event_loop);
			}
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
