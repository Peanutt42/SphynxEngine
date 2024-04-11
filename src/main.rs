use sphynx_logging::*;
use sphynx_rendering::Renderer;
use winit::{
	event_loop::EventLoop,
	window::WindowBuilder,
	event::{Event, WindowEvent}
};
use std::sync::Arc;

fn main() -> anyhow::Result<()> {
	sphynx_logging::init();

    info!(General, "=== INITIALIZING SPHYNXENGINE ===");

    let event_loop = EventLoop::new()?;
	let window = Arc::new(WindowBuilder::new().build(&event_loop)?);
	let window_clone = window.clone();
	let mut renderer = pollster::block_on(Renderer::new(&window))?;

	event_loop.run(move |event, target| {
		if window_clone.has_focus() {
			window_clone.request_redraw();
		}

		if let Event::WindowEvent { event, .. } = event {
			match event {
				WindowEvent::Resized(new_size) => renderer.resize(new_size),
				WindowEvent::RedrawRequested => {
					// update

					renderer.update();
				},
				WindowEvent::CloseRequested => target.exit(),
				_ => {}
			}
		}
	})?;

    info!(General, "=== SHUTTING SPHYNXENGINE DOWN ===");

    Ok(())
}
