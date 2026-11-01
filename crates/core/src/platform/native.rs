use crate::Engine;
use anyhow::Result;
use sphynx_logging::info;
use sphynx_rendering::Renderer;
use std::sync::Arc;
use winit::{
	event_loop::EventLoop,
	window::{Fullscreen, Window, WindowAttributes},
};

pub fn run<G: 'static, S: 'static>(engine: Engine<G, S>, event_loop: EventLoop<()>) -> Result<()> {
	let mut engine = engine;
	event_loop.run_app(&mut engine)?;

	info!(General, "=== SHUTDOWN ===");

	Ok(())
}

pub fn configure_window(attributes: WindowAttributes, fullscreen: bool) -> WindowAttributes {
	attributes.with_fullscreen(if fullscreen {
		Some(Fullscreen::Borderless(None))
	} else {
		None
	})
}

pub struct RendererInit {
	result: Option<Result<Renderer>>,
}
impl RendererInit {
	pub fn start(window: Arc<Window>, vsync: bool) -> Self {
		Self {
			result: Some(pollster::block_on(Renderer::new(window, vsync))),
		}
	}
	pub fn poll(&mut self) -> Option<Result<Renderer>> {
		self.result.take()
	}
}
