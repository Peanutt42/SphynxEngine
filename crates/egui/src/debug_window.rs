pub fn debug_window(ctx: &egui::Context, dt: f32, game_version: u64, show_inspector: &mut bool) {
	egui::Window::new(egui::RichText::new("[DEBUG]").strong())
		.resizable(false)
		.default_open(false)
		.show(ctx, |ui| {
			ui.label(format!(
				"FPS: {} ({:.2} ms) Game Version: {game_version}",
				(1.0 / dt).round(),
				dt * 1000.0
			));

			ui.separator();

			if ui
				.button(if *show_inspector {
					"Hide inspector"
				} else {
					"Show inspector"
				})
				.clicked()
			{
				*show_inspector = !*show_inspector;
			}
		});
}
