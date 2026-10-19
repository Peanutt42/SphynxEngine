//! Mostly copied from <https://github.com/kaphula/winit-egui-wgpu-template/blob/5b5ffc9eaf561e688f7864a9f79c8c33da7487f5/src/egui_tools.rs>
//! Since this "template" is not really up to date with the newest egui version,
//! the TexturesDelta uploading/painting was copied from
//! <https://github.com/emilk/egui/blob/86099aca1276fe786921992c8d5a68914ee79bec/crates/egui-wgpu/src/winit.rs#L568-L578>

use egui::Context;
use egui_wgpu::wgpu::{
	CommandEncoder, Device, Queue, StoreOp, SurfaceConfiguration, TextureFormat, TextureView,
};
use egui_wgpu::{wgpu, Renderer, RendererOptions, ScreenDescriptor};
use egui_winit::State;
use std::sync::Arc;
use winit::event::WindowEvent;
use winit::window::Window;

pub struct EguiRenderer {
	state: State,
	renderer: Renderer,
	window: Arc<Window>,
	frame_started: bool,
	scale_factor: f32,
	last_update: std::time::Instant,
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
			last_update: std::time::Instant::now(),
		}
	}

	pub fn handle_input(&mut self, window: &Window, event: &WindowEvent) {
		let _ = self.state.on_window_event(window, event);
	}

	pub fn ppp(&mut self, v: f32) {
		self.context().set_pixels_per_point(v);
	}

	pub fn begin_frame(&mut self) {
		let dt = self.last_update.elapsed().as_secs_f32();
		self.last_update = std::time::Instant::now();

		let raw_input = self.state.take_egui_input(self.window.as_ref());
		self.state.egui_ctx().begin_pass(raw_input);
		self.frame_started = true;

		// TODO: move out!!!
		egui::Window::new("egui is working!")
			.resizable(true)
			.vscroll(true)
			.default_open(true)
			.show(self.state.egui_ctx(), |ui| {
				ui.label(format!(
					"FPS: {} ({:.2} ms)",
					(1.0 / dt).round(),
					dt * 1000.0
				));

				ui.separator();

				if ui.button("Button!").clicked() {
					println!("Button clicked!")
				}

				ui.separator();

				ui.horizontal(|ui| {
					ui.label(format!("Pixels per point: {}", ui.pixels_per_point()));
					if ui.button("-").clicked() {
						self.scale_factor = (self.scale_factor - 0.1).max(0.3);
					}
					if ui.button("+").clicked() {
						self.scale_factor = (self.scale_factor + 0.1).min(3.0);
					}
				});
			});
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
