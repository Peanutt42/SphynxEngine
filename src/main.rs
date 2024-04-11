use sphynx_logging::*;
use sphynx_rendering::Renderer;
use winit::{
	event_loop::EventLoop,
	window::WindowBuilder,
	event::{Event, WindowEvent}
};

fn main() -> anyhow::Result<()> {
	sphynx_logging::init();

    info!(General, "=== INITIALIZING SPHYNXENGINE ===");

    let event_loop = EventLoop::new()?;
	let window = WindowBuilder::new().build(&event_loop)?;
	let mut renderer = pollster::block_on(Renderer::new(&window))?;

	event_loop.run(move |event, target| {
		target.set_control_flow(winit::event_loop::ControlFlow::Poll);

		if let Event::WindowEvent { event, .. } = event {
			match event {
				WindowEvent::Resized(new_size) => renderer.resize(new_size),
				WindowEvent::RedrawRequested => renderer.update(),
				WindowEvent::CloseRequested => target.exit(),
				_ => {}
			}
		}

	})?;

    info!(General, "=== SHUTTING SPHYNXENGINE DOWN ===");

    Ok(())
}
