use winit::{dpi::PhysicalPosition, event::{ElementState, KeyEvent, MouseButton}, keyboard::{KeyCode, PhysicalKey}};
use cgmath::{Vector2, Zero};
use std::collections::HashMap;


pub struct Input {
	pub mouse_delta: Vector2<f32>,
	pub cursor_position: PhysicalPosition<f64>,
	key_states: HashMap<KeyCode, bool>,
	last_frame_key_states: HashMap<KeyCode, bool>,
	mouse_button_states: HashMap<MouseButton, bool>,
	last_frame_mouse_button_states: HashMap<MouseButton, bool>,
}

impl Input {
	pub fn new() -> Self {
		Self {
			mouse_delta: Vector2::zero(),
			cursor_position: PhysicalPosition::new(0.0, 0.0),
			key_states: HashMap::new(),
			last_frame_key_states: HashMap::new(),
			mouse_button_states: HashMap::new(),
			last_frame_mouse_button_states: HashMap::new(),
		}
	}

	pub fn is_key_down(&self, key_code: KeyCode) -> bool {
		self.key_states.get(&key_code).copied().unwrap_or(false)
	}

	/// only when key goes from a released to a pressed state
	pub fn was_key_pressed(&self, key_code: KeyCode) -> bool {
		let last_state = self.last_frame_key_states.get(&key_code).copied().unwrap_or(false);
		if let Some(state) = self.key_states.get(&key_code) {
			return !last_state && *state;
		}
		false
	}

	pub fn is_mouse_button_down(&self, button: MouseButton) -> bool {
		self.mouse_button_states.get(&button).copied().unwrap_or(false)
	}

	/// only when mouse button goes from a released to a pressed state
	pub fn was_mouse_button_pressed(&self, button: MouseButton) -> bool {
		let last_state = self.last_frame_mouse_button_states.get(&button).copied().unwrap_or(false);
		if let Some(state) = self.mouse_button_states.get(&button) {
			return !last_state && *state;
		}
		false
	}

	/// only when mouse button goes from a pressed to a released state
	pub fn was_mouse_button_released(&self, button: MouseButton) -> bool {
		if let Some(last_state) = self.last_frame_mouse_button_states.get(&button) {
			if let Some(state) = self.mouse_button_states.get(&button) {
				return *last_state && !*state;
			}
		}
		false
	}

	pub fn handle_keyboard(&mut self, keyboard_event: KeyEvent) {
		if let PhysicalKey::Code(key_code) = keyboard_event.physical_key {
			match keyboard_event.state {
				ElementState::Pressed => { self.key_states.insert(key_code, true); },
				ElementState::Released => { self.key_states.insert(key_code, false); },
			}
		}
	}

	pub fn handle_mouse(&mut self, button: MouseButton, state: ElementState) {
		match state {
			ElementState::Pressed => { self.mouse_button_states.insert(button, true); },
			ElementState::Released => { self.mouse_button_states.insert(button, false); },
		}
	}

	pub fn handle_mouse_movement(&mut self, delta: (f64, f64)) {
		self.mouse_delta.x = delta.0 as f32;
		self.mouse_delta.y = delta.1 as f32;
	}

	pub fn handle_cursor_movement(&mut self, new_position: PhysicalPosition<f64>) {
		self.cursor_position = new_position;
	}

	pub fn clear_frame(&mut self) {
		self.mouse_delta = Vector2::zero();
		self.last_frame_key_states.clone_from(&self.key_states);
		self.last_frame_mouse_button_states.clone_from(&self.mouse_button_states);
	}
}

impl Default for Input {
	fn default() -> Self {
		Self::new()
	}
}