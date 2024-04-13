use sphynx_logging::*;
use sphynx_rendering::{Renderer, Transform};
use winit::{
	event_loop::EventLoop,
	window::{WindowBuilder, Window},
	event::{Event, WindowEvent}
};
use cgmath::Vector3;
use std::time::Instant;
use std::sync::Arc;

pub struct EngineConfig {

}

impl EngineConfig {
	pub fn new() -> Self {
		Self {

		}
	}

	pub fn run(self) -> anyhow::Result<()> {
		let event_loop = EventLoop::new()?;
		let engine = Engine::new(&event_loop)?;
		engine.run(event_loop)
	}
}

impl Default for EngineConfig {
	fn default() -> Self {
		Self::new()
	}
}

struct Engine {
	window: Arc<Window>,
	renderer: Renderer,
	start_time: Instant,
}

impl Engine {
	fn new(event_loop: &EventLoop<()>) -> anyhow::Result<Self> {
		let window = Arc::new(WindowBuilder::new().build(event_loop)?);
		let renderer = pollster::block_on(Renderer::new(window.clone()))?;

		Ok(Self {
			window,
			renderer,
			start_time: Instant::now(),
		})
	}

	pub fn run(mut self, event_loop: EventLoop<()>) -> anyhow::Result<()> {
		sphynx_logging::init();

		info!(General, "=== INITIALIZING ===");
	
		{
			event_loop.run(move |event, target| {
				self.window.request_redraw();
	
				if let Event::WindowEvent { event, .. } = event {
					match event {
						WindowEvent::Resized(new_size) => self.renderer.resize(new_size),
						WindowEvent::RedrawRequested => self.update(),
						WindowEvent::CloseRequested => target.exit(),
						_ => {}
					}
				}
			})?;
		}
	
		info!(General, "=== SHUTDOWN ===");

		Ok(())
	}

	fn update(&mut self) {
		let time = (Instant::now() - self.start_time).as_secs_f32();
		for (i, instance) in self.renderer.instances.iter_mut().enumerate() {
			*instance = Transform::new(Vector3::new(i as f32 * 0.1, 0.5 * f32::sin(time + 0.2 * i as f32), 0.5 + 0.5 * f32::cos(time + 0.2 * i as f32)));
		}

		self.renderer.update();
	}
}