use winit::{event::MouseButton,keyboard::KeyCode};
use cgmath::Rad;
use sphynx_rendering::Camera;
use sphynx_input::Input;

use std::f32::consts::FRAC_PI_2;
const SAFE_FRAC_PI_2: f32 = FRAC_PI_2 - f32::EPSILON;

pub struct CameraController {
	speed: f32,
	sensitivity: f32,
}

impl CameraController {
	pub fn new(speed: f32, sensitivity: f32) -> Self {
		Self {
			speed,
			sensitivity,
		}
	}

	pub fn update(&mut self, input: &Input, camera: &mut Camera, dt: f32) {
		if input.is_mouse_button_down(MouseButton::Right) {
			camera.yaw += Rad(input.mouse_delta.x) * self.sensitivity * dt;
			camera.pitch -= Rad(input.mouse_delta.y) * self.sensitivity * dt;

			let mut speed = self.speed;
			if input.is_key_down(KeyCode::ShiftLeft) {
				speed *= 2.0;
			}
			if input.is_key_down(KeyCode::ControlLeft) {
				speed *= 0.5;
			}

			let mut amount_forward = if input.is_key_down(KeyCode::KeyW) { 1.0 } else { 0.0 };
			if input.is_key_down(KeyCode::KeyS) { amount_forward -= 1.0; }
			camera.position += camera.get_forward_direction() * amount_forward * speed * dt;

			let mut amount_right = if input.is_key_down(KeyCode::KeyA) { -1.0 } else { 0.0 };
			if input.is_key_down(KeyCode::KeyD) { amount_right += 1.0; }
			camera.position += camera.get_right_direction() * amount_right * speed * dt;

			let mut amount_up = if input.is_key_down(KeyCode::KeyE) { 1.0 } else { 0.0 };
			if input.is_key_down(KeyCode::KeyQ) { amount_up -= 1.0; }
			camera.position += camera.get_up_direction() * amount_up * speed * dt;
		}

		// clamp pitch
		if camera.pitch < -Rad(SAFE_FRAC_PI_2) {
			camera.pitch = -Rad(SAFE_FRAC_PI_2);
		}
		else if camera.pitch > Rad(SAFE_FRAC_PI_2) {
			camera.pitch = Rad(SAFE_FRAC_PI_2);
		}
	}
}
