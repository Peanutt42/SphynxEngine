use crate::egui;
use egui::{Align2, AtomExt, Atoms};
use hecs::{Entity, World};
use sphynx_hecs_component::reflection::{ValueMut, VisitorMut};
use sphynx_hecs_component::ComponentType;
use std::collections::HashSet;

/// Persistent UI state for [`entity_inspector`].
#[derive(Default)]
pub struct InspectorState {
	pub selected: Option<Entity>,
	pub filter: String,
	pub collapsed: HashSet<String>,
	pub add_component_filter: String,
}

/// Draws the ECS inspector window for the given world.
pub fn entity_inspector(
	ctx: &egui::Context,
	world: &mut World,
	component_types: &[&'static ComponentType],
	state: &mut InspectorState,
) {
	egui::Window::new("ECS Inspector")
		.resizable(true)
		.default_size(egui::vec2(600.0, 500.0))
		.show(ctx, |ui| {
			ui.horizontal(|ui| {
				ui.heading("Entities");
				ui.label(egui::RichText::new(format!("({})", world.len())).weak());
			});
			ui.add_space(4.0);

			ui.horizontal(|ui| {
				ui.label("Filter:");
				ui.add(
					egui::TextEdit::singleline(&mut state.filter)
						.hint_text("Search entities...")
						.desired_width(ui.available_width()),
				);
			});
			if !state.filter.is_empty() {
				if ui.small_button("Clear").clicked() {
					state.filter.clear();
				}
				ui.add_space(4.0);
			}

			ui.separator();
			ui.add_space(4.0);

			let rows = filtered_entities(world, &state.filter.to_lowercase());

			egui::Panel::left("ecs_entity_list")
				.resizable(true)
				.size_range(egui::Rangef::new(200.0, 400.0))
				.show(ui, |ui| {
					entity_list(ui, &rows, state);
				});

			egui::CentralPanel::default().show(ui, |ui| {
				component_panel(ui, world, component_types, state);
			});
		});
}

fn filtered_entities(world: &World, filter: &str) -> Vec<Entity> {
	let mut rows = world
		.iter()
		.filter_map(|entity_ref| {
			let entity = entity_ref.entity();
			(filter.is_empty() || format!("{entity:?}").to_lowercase().contains(filter))
				.then_some(entity)
		})
		.collect::<Vec<_>>();

	rows.sort_by_key(|entity| entity.id());
	rows
}

fn entity_list(ui: &mut egui::Ui, rows: &[Entity], state: &mut InspectorState) {
	let row_height = ui.spacing().interact_size.y + 2.0;

	egui::ScrollArea::vertical()
		.id_salt("ecs_inspector_entity_list")
		//.auto_shrink([false, false])
		.show_rows(ui, row_height, rows.len(), |ui, visible_rows| {
			for &entity in &rows[visible_rows] {
				let selected = state.selected == Some(entity);
				let label = format!("{entity:?}");

				if ui
					.add_sized(
						egui::Vec2::new(ui.available_width(), row_height),
						egui::Button::selectable(selected, Atoms::default())
							.left_text(egui::RichText::new(label).atom_align(Align2::LEFT_CENTER)),
					)
					.clicked()
				{
					state.selected = Some(entity);
					state.add_component_filter.clear();
				}
				ui.add_space(1.0);
			}
		});
}

fn component_panel(
	ui: &mut egui::Ui,
	world: &mut World,
	component_types: &[&'static ComponentType],
	state: &mut InspectorState,
) {
	let Some(entity) = state.selected else {
		ui.vertical_centered(|ui| {
			ui.add_space(60.0);
			ui.label(egui::RichText::new("Select an entity to inspect").weak());
		});
		return;
	};

	if !world.contains(entity) {
		state.selected = None;
		state.add_component_filter.clear();
		ui.vertical_centered(|ui| {
			ui.add_space(60.0);
			ui.label(egui::RichText::new("Entity no longer exists").weak());
		});
		return;
	}

	let mut remove_entity = false;
	ui.horizontal(|ui| {
		ui.heading(egui::RichText::new(format!("{entity:?}")).strong());
		ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
			if ui
				.button(egui::RichText::new("Remove Entity").strong())
				.clicked()
			{
				remove_entity = true;
			}

			add_component_dropdown(ui, world, entity, component_types, state);
		});
	});
	if remove_entity {
		let _ = world.despawn(entity);
		state.selected = None;
		state.add_component_filter.clear();
		return;
	}
	ui.add_space(2.0);
	ui.separator();
	ui.add_space(6.0);

	egui::ScrollArea::vertical()
		.id_salt("ecs_inspector_component_list")
		.max_height(ui.available_height().max(0.0))
		.min_scrolled_height(0.0)
		.auto_shrink([false, false])
		.show(ui, |ui| {
			for component_type in component_types {
				if !(component_type.has_entity)(world, entity) {
					continue;
				}
				let mut visitor = ComponentUi::new(ui, &mut state.collapsed);
				(component_type.visit_mut)(world, entity, &mut visitor);
				if visitor.removed {
					(component_type.remove)(world, entity);
					break;
				}
				ui.add_space(10.0);
			}
			if world.entity(entity).map_or(true, |e| e.is_empty()) {
				ui.vertical_centered(|ui| {
					ui.add_space(40.0);
					ui.label(egui::RichText::new("No components").weak());
				});
			}
		});
}

fn add_component_dropdown(
	ui: &mut egui::Ui,
	world: &mut World,
	entity: Entity,
	component_types: &[&'static ComponentType],
	state: &mut InspectorState,
) {
	let button = egui::Button::new(egui::RichText::new("+ Add Component").strong());
	let resp = ui.add(button);
	let popup_id = egui::Popup::default_response_id(&resp);
	if resp.clicked() {
		state.add_component_filter.clear();
	}

	egui::Popup::menu(&resp)
		.close_behavior(egui::PopupCloseBehavior::CloseOnClickOutside)
		.show(|ui| {
			ui.set_width(300.0);
			let filter_resp = ui.add(
				egui::TextEdit::singleline(&mut state.add_component_filter)
					.hint_text("Search...")
					.desired_width(ui.available_width()),
			);
			filter_resp.request_focus();
			ui.add_space(4.0);
			ui.separator();
			ui.add_space(4.0);

			let filter = state.add_component_filter.to_lowercase();
			let mut available = 0;

			egui::ScrollArea::vertical()
				.max_height(240.0)
				.show(ui, |ui| {
					for component_type in component_types {
						let name = (component_type.type_name)();
						if !filter.is_empty() && !name.to_lowercase().contains(&filter) {
							continue;
						}
						if (component_type.has_entity)(world, entity) {
							continue;
						}
						available += 1;
						if ui.selectable_label(false, name).clicked() {
							(component_type.insert_default)(world, entity);
							egui::Popup::close_id(ui.ctx(), popup_id);
							state.add_component_filter.clear();
						}
					}
					if available == 0 {
						ui.label(egui::RichText::new("No components available").weak());
					}
				});
		});
}

struct ComponentUi<'a> {
	ui: &'a mut egui::Ui,
	frame: Option<egui::containers::frame::Prepared>,
	collapsed: &'a mut HashSet<String>,
	depth: usize,
	visible: bool,
	pending_field: Option<&'static str>,
	removed: bool,
}

impl<'a> ComponentUi<'a> {
	fn new(ui: &'a mut egui::Ui, collapsed: &'a mut HashSet<String>) -> Self {
		Self {
			ui,
			frame: None,
			collapsed,
			depth: 0,
			visible: true,
			pending_field: None,
			removed: false,
		}
	}

	/// The ui fields should be drawn into: the inside of the component frame when
	/// one is open, otherwise the parent ui.
	fn content_ui(&mut self) -> &mut egui::Ui {
		if let Some(frame) = &mut self.frame {
			&mut frame.content_ui
		} else {
			&mut *self.ui
		}
	}

	fn indent(&self) -> f32 {
		self.depth as f32 * 16.0
	}

	/// Starts a nested group (struct, tuple or sequence).
	fn begin_group(&mut self, type_name: &str) {
		if !self.visible || self.removed {
			return;
		}
		let pending_field = self.pending_field.take();
		let field_name = match &pending_field {
			Some(field) => field,
			None => type_name,
		};
		let indent = self.indent();
		let ui = self.content_ui();
		ui.horizontal(|ui| {
			ui.add_space(indent);
			ui.label(egui::RichText::new(field_name).strong());
			if pending_field.is_some() {
				ui.label(egui::RichText::new(type_name).weak());
			}
		});
		self.depth += 1;
	}

	fn end_group(&mut self) {
		if self.visible {
			self.depth = self.depth.saturating_sub(1);
		}
	}
}

impl VisitorMut for ComponentUi<'_> {
	fn begin_component(&mut self, name: &'static str, _field_count: usize) {
		let key = format!("comp::{name}");
		let mut open = !self.collapsed.contains(&key);

		// The outline wraps the header *and* the fields so components are easy to
		// tell apart at a glance.
		let frame = egui::Frame::new()
			.fill(self.ui.style().visuals.extreme_bg_color)
			.stroke(self.ui.style().visuals.widgets.noninteractive.bg_stroke)
			.corner_radius(6.0)
			.inner_margin(egui::Margin::symmetric(8, 6));
		self.frame = Some(frame.begin(self.ui));

		let (clicked_header, clicked_remove) = {
			let ui = self.content_ui();
			egui::Sides::new().shrink_left().show(
				ui,
				|ui| {
					let marker = if open { "v" } else { ">" };
					ui.add_sized(
						[ui.available_width(), ui.spacing().interact_size.y],
						egui::Button::new((
							egui::RichText::new(format!("{marker} {name}")).strong(),
							egui::Atom::grow(),
						))
						.frame(false),
					)
					.clicked()
				},
				|ui| {
					ui.small_button("x")
						.on_hover_text("Remove component")
						.clicked()
				},
			)
		};

		if clicked_remove {
			self.removed = true;
			self.visible = false;
			return;
		}
		if clicked_header {
			open = !open;
			if open {
				self.collapsed.remove(&key);
			} else {
				self.collapsed.insert(key);
			}
		}

		self.visible = open;
		self.pending_field = None;
		if open {
			self.content_ui().add_space(4.0);
			self.depth = 1;
		} else {
			self.depth = 0;
		}
	}

	fn end_component(&mut self, _name: &'static str) {
		if let Some(frame) = self.frame.take() {
			frame.end(self.ui);
		}
		self.visible = false;
		self.depth = 0;
		self.pending_field = None;
	}

	fn begin_struct(&mut self, name: &'static str, _field_count: usize) {
		self.begin_group(name);
	}

	fn end_struct(&mut self, _name: &'static str) {
		self.end_group();
	}

	fn begin_tuple(&mut self, type_name: &'static str, _len: usize) {
		self.begin_group(type_name);
	}

	fn end_tuple(&mut self, _type_name: &'static str) {
		self.end_group();
	}

	fn begin_field(&mut self, name: &'static str) {
		self.pending_field = Some(name);
	}

	fn end_field(&mut self, _name: &'static str) {}

	fn begin_seq(&mut self, type_name: &'static str, _len: usize) {
		self.begin_group(type_name);
	}

	fn end_seq(&mut self, _type_name: &'static str) {
		self.end_group();
	}

	fn visit_value(&mut self, value: ValueMut<'_>) {
		if !self.visible || self.removed {
			return;
		}

		let field = self.pending_field.take();
		let indent = self.indent();

		let ui = self.content_ui();
		ui.horizontal(|ui| {
			ui.add_space(indent);
			if let Some(field) = field {
				ui.label(egui::RichText::new(format!("{field}:")).strong());
			}

			match value {
				ValueMut::Bool(value) => {
					let _ = ui.checkbox(value, "");
				}
				ValueMut::I16(value) => {
					let _ = ui.add(egui::DragValue::new(value).speed(1));
				}
				ValueMut::U16(value) => {
					let _ = ui.add(egui::DragValue::new(value).speed(1));
				}
				ValueMut::I32(value) => {
					let _ = ui.add(egui::DragValue::new(value).speed(1));
				}
				ValueMut::U32(value) => {
					let _ = ui.add(egui::DragValue::new(value).speed(1));
				}
				ValueMut::I64(value) => {
					let _ = ui.add(egui::DragValue::new(value).speed(1));
				}
				ValueMut::U64(value) => {
					let _ = ui.add(egui::DragValue::new(value).speed(1));
				}
				ValueMut::I128(value) => {
					ui.add_enabled(
						false,
						egui::TextEdit::singleline(&mut value.to_string()).desired_width(140.0),
					);
				}
				ValueMut::U128(value) => {
					ui.add_enabled(
						false,
						egui::TextEdit::singleline(&mut value.to_string()).desired_width(140.0),
					);
				}
				ValueMut::F32(value) => {
					let _ = ui.add(egui::DragValue::new(value).speed(0.01));
				}
				ValueMut::F64(value) => {
					let _ = ui.add(egui::DragValue::new(value).speed(0.001));
				}
				ValueMut::Str(value) => {
					let _ = ui.label(egui::RichText::new(value.to_string()).strong());
				}
				ValueMut::String(value) => {
					let _ = ui.add(egui::TextEdit::singleline(value).desired_width(180.0));
				}
				ValueMut::Enum(mut value) => {
					let variants = value.variants;
					egui::ComboBox::from_id_salt(ui.next_auto_id())
						.selected_text(variants[value.selected])
						.show_ui(ui, |ui| {
							for (index, variant) in variants.iter().enumerate() {
								if ui
									.selectable_label(index == value.selected, *variant)
									.clicked()
								{
									value.set(index);
								}
							}
						});
				}
			}
		});
	}
}
