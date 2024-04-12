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

    info!(General, "=== INITIALIZING ===");

	{
		let event_loop = EventLoop::new()?;
		let window = Arc::new(WindowBuilder::new().build(&event_loop)?);
		let mut renderer = pollster::block_on(Renderer::new(window.clone()))?;

		event_loop.run(move |event, target| {
			window.request_redraw();

			if let Event::WindowEvent { event, .. } = event {
				match event {
					WindowEvent::Resized(new_size) => renderer.resize(new_size),
					WindowEvent::RedrawRequested => renderer.update(),
					WindowEvent::CloseRequested => target.exit(),
					_ => {}
				}
			}
		})?;
	}

    info!(General, "=== SHUTDOWN ===");

    Ok(())
}
