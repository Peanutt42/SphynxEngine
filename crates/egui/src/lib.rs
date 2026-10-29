//! Mostly copied from <https://github.com/kaphula/winit-egui-wgpu-template/blob/5b5ffc9eaf561e688f7864a9f79c8c33da7487f5/src/egui_tools.rs>
//! Since this "template" is not really up to date with the newest egui version,
//! the TexturesDelta uploading/painting was copied from
//! <https://github.com/emilk/egui/blob/86099aca1276fe786921992c8d5a68914ee79bec/crates/egui-wgpu/src/winit.rs#L568-L578>

use egui::text_selection::LabelSelectionState;
use egui::Context;
use egui_wgpu::wgpu::{
	CommandEncoder, Device, Queue, StoreOp, SurfaceConfiguration, TextureFormat, TextureView,
};
use egui_wgpu::{wgpu, Renderer, RendererOptions, ScreenDescriptor};
use egui_winit::State;
use std::sync::Arc;
use winit::event::WindowEvent;
use winit::window::Window;

/// reexport
pub use egui;

mod debug_window;
pub use debug_window::debug_window;

mod entity_inspector;
pub use entity_inspector::{entity_inspector, InspectorState};

/// Clears egui's multi-widget label text selection.
///
/// egui only ends a label-selection drag when it observes a pointer release. If
/// that release is missed (for example when the window loses focus or the
/// pointer leaves the window mid-drag), the selection keeps following the
/// pointer and clicking on other text does not reliably stop it. Dropping the
/// selection also resets the internal "is dragging" flag that causes this.
fn clear_label_selection(ctx: &Context) {
	if let Some(state) = ctx.plugin_opt::<LabelSelectionState>() {
		state.lock().clear_selection();
	}
}

pub struct EguiRenderer {
	state: State,
	renderer: Renderer,
	window: Arc<Window>,
	frame_started: bool,
	scale_factor: f32,
}

impl EguiRenderer {
	pub fn context(&self) -> &Context {
		self.state.egui_ctx()
	}

	pub fn new(
		device: &Device,
		output_color_format: TextureFormat,
		output_depth_format: Option<TextureFormat>,
		msaa_samples: u32,
		window: Arc<Window>,
		scale_factor: f32,
	) -> EguiRenderer {
		let egui_context = Context::default();

		let egui_state = egui_winit::State::new(
			egui_context,
			egui::viewport::ViewportId::ROOT,
			&window,
			Some(window.scale_factor() as f32),
			None,
			Some(2 * 1024), // default dimension is 2048
		);
		let egui_renderer = Renderer::new(
			device,
			output_color_format,
			RendererOptions {
				msaa_samples,
				depth_stencil_format: output_depth_format,
				dithering: true,
				..Default::default()
			},
		);

		EguiRenderer {
			state: egui_state,
			renderer: egui_renderer,
			window,
			frame_started: false,
			scale_factor,
		}
	}

	pub fn handle_input(&mut self, window: &Window, event: &WindowEvent) {
		let _ = self.state.on_window_event(window, event);

		// A drag that is aborted by the window losing focus or the pointer
		// leaving the window will never receive its release event. Clear the
		// label selection so it cannot get stuck and follow the pointer.
		let aborted = matches!(
			event,
			WindowEvent::Focused(false) | WindowEvent::CursorLeft { .. }
		);
		let pointer_down = self
			.state
			.egui_ctx()
			.input(|input| input.pointer.any_down());
		if aborted && pointer_down {
			clear_label_selection(self.state.egui_ctx());
		}
	}

	pub fn ppp(&mut self, v: f32) {
		self.context().set_pixels_per_point(v);
	}

	pub fn begin_frame(&mut self) {
		let raw_input = self.state.take_egui_input(self.window.as_ref());
		let ctx = self.state.egui_ctx();
		ctx.begin_pass(raw_input);

		// A new press always begins a fresh selection gesture. Clearing here
		// (rather than waiting for egui's own bookkeeping) makes the previous
		// highlight disappear immediately and recovers from a missed release.
		if ctx.input(|input| input.pointer.any_pressed() && !input.modifiers.shift) {
			clear_label_selection(ctx);
		}

		self.frame_started = true;
	}

	pub fn end_frame_and_draw(
		&mut self,
		device: &Device,
		queue: &Queue,
		encoder: &mut CommandEncoder,
		window_surface_view: &TextureView,
		surface_config: &SurfaceConfiguration,
	) {
		if !self.frame_started {
			panic!("begin_frame must be called before end_frame_and_draw can be called!");
		}

		let screen_descriptor = ScreenDescriptor {
			size_in_pixels: [surface_config.width, surface_config.height],
			pixels_per_point: self.window.as_ref().scale_factor() as f32 * self.scale_factor,
		};

		self.ppp(screen_descriptor.pixels_per_point);

		let mut full_output = self.state.egui_ctx().end_pass();

		self.state
			.handle_platform_output(self.window.as_ref(), full_output.platform_output);

		let tris = self
			.state
			.egui_ctx()
			.tessellate(full_output.shapes, self.state.egui_ctx().pixels_per_point());

		#[expect(clippy::iter_over_hash_type)] // Order doesn't matter here
		for (id, image_deltas) in full_output.textures_delta.set.drain() {
			for image_delta in image_deltas {
				self.renderer
					.update_texture(device, queue, id, &image_delta);
			}
		}

		self.renderer
			.update_buffers(device, queue, encoder, &tris, &screen_descriptor);

		let rpass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
			color_attachments: &[Some(wgpu::RenderPassColorAttachment {
				view: window_surface_view,
				resolve_target: None,
				ops: egui_wgpu::wgpu::Operations {
					load: egui_wgpu::wgpu::LoadOp::Load,
					store: StoreOp::Store,
				},
				depth_slice: None,
			})],
			depth_stencil_attachment: None,
			timestamp_writes: None,
			label: Some("egui main render pass"),
			occlusion_query_set: None,
			multiview_mask: None,
		});

		self.renderer
			.render(&mut rpass.forget_lifetime(), &tris, &screen_descriptor);

		for x in full_output.textures_delta.free.iter() {
			self.renderer.free_texture(x)
		}

		self.frame_started = false;
	}
}
