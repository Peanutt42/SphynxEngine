use winit::{event::{ElementState, KeyEvent, MouseButton}, keyboard::{KeyCode, PhysicalKey}};
use cgmath::{Vector2, Zero};
use std::collections::HashMap;


pub struct Input {
	pub mouse_delta: Vector2<f32>,
	key_states: HashMap<KeyCode, bool>,
	mouse_button_states: HashMap<MouseButton, bool>,
}

impl Input {
	pub fn new() -> Self {
		Self {
			mouse_delta: Vector2::zero(),
			key_states: HashMap::new(),
			mouse_button_states: HashMap::new(),
		}
	}

	pub fn clear_frame(&mut self) {
		self.mouse_delta = Vector2::zero();
	}

	pub fn is_mouse_pressed(&self, button: MouseButton) -> bool {
		self.mouse_button_states.get(&button).copied().unwrap_or(false)
	}

	pub fn is_key_pressed(&self, key_code: KeyCode) -> bool {
		self.key_states.get(&key_code).copied().unwrap_or(false)
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
}

impl Default for Input {
	fn default() -> Self {
		Self::new()
	}
}