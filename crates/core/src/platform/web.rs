use crate::Engine;
use anyhow::Result;
use sphynx_rendering::Renderer;
use std::{cell::RefCell, rc::Rc, sync::Arc};
use winit::{
	event_loop::EventLoop,
	platform::web::{EventLoopExtWebSys, WindowAttributesExtWebSys},
	window::{Window, WindowAttributes},
};

pub fn run<G: 'static, S: 'static>(engine: Engine<G, S>, event_loop: EventLoop<()>) -> Result<()> {
	event_loop.spawn_app(engine);
	Ok(())
}

pub fn configure_window(attributes: WindowAttributes, _fullscreen: bool) -> WindowAttributes {
	attributes
		.with_append(true)
		.with_focusable(true)
		.with_prevent_default(true)
}

/// holds reference to the result of the async task that creates the renderer
pub struct RendererInit {
	result: Rc<RefCell<Option<Result<Renderer>>>>,
}
impl RendererInit {
	pub fn start(window: Arc<Window>, vsync: bool) -> Self {
		let result = Rc::new(RefCell::new(None));
		let task_result = Rc::clone(&result);
		let task_window = Arc::clone(&window);

		wasm_bindgen_futures::spawn_local(async move {
			*task_result.borrow_mut() = Some(Renderer::new(Arc::clone(&task_window), vsync).await);
			// Wake the event loop so it can install the renderer.
			task_window.request_redraw();
		});

		Self { result }
	}
	pub fn poll(&mut self) -> Option<Result<Renderer>> {
		self.result.borrow_mut().take()
	}
}
