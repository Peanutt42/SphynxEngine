pub fn debug_window(ctx: &egui::Context, dt: f32, game_version: u64) {
	egui::Window::new("egui is working!")
		.resizable(true)
		.vscroll(true)
		.default_open(true)
		.show(ctx, |ui| {
			ui.label(format!(
				"FPS: {} ({:.2} ms) Game Version: {game_version}",
				(1.0 / dt).round(),
				dt * 1000.0
			));

			ui.separator();

			if ui.button("Button!").clicked() {
				println!("Button clicked!")
			}

			/*
			ui.separator();

			ui.horizontal(|ui| {
			ui.label(format!("Pixels per point: {}", ui.pixels_per_point()));
			if ui.button("-").clicked() {
			egui_renderer.scale_factor = (egui_renderer.scale_factor - 0.1).max(0.3);
			}
			if ui.button("+").clicked() {
			egui_renderer.scale_factor = (egui_renderer.scale_factor + 0.1).min(3.0);
			}
			});
			*/
		});
}
