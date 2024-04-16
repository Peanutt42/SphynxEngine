use winit::event::*;
use winit::keyboard::KeyCode;
use cgmath::{InnerSpace, Vector3, Rad};
use sphynx_rendering::Camera;
use sphynx_input::Input;

use std::f32::consts::FRAC_PI_2;
const SAFE_FRAC_PI_2: f32 = FRAC_PI_2 - 0.0001;

pub struct CameraController {
	amount_left: f32,
	amount_right: f32,
	amount_forward: f32,
	amount_backward: f32,
	amount_up: f32,
	amount_down: f32,
	rotate_horizontal: f32,
	rotate_vertical: f32,
	speed: f32,
	sensitivity: f32,
}

impl CameraController {
	pub fn new(speed: f32, sensitivity: f32) -> Self {
		Self {
			amount_left: 0.0,
			amount_right: 0.0,
			amount_forward: 0.0,
			amount_backward: 0.0,
			amount_up: 0.0,
			amount_down: 0.0,
			rotate_horizontal: 0.0,
			rotate_vertical: 0.0,
			speed,
			sensitivity,
		}
	}

	pub fn process_input(&mut self, input: &Input) {
		let enabled = input.is_mouse_pressed(MouseButton::Right);

		if enabled {
			self.rotate_horizontal = input.mouse_delta.x;
			self.rotate_vertical = input.mouse_delta.y;
		}

		self.amount_forward = if enabled && input.is_key_pressed(KeyCode::KeyW) { 1.0 } else { 0.0 };
		self.amount_backward = if enabled && input.is_key_pressed(KeyCode::KeyS) { 1.0 } else { 0.0 };
		self.amount_left = if enabled && input.is_key_pressed(KeyCode::KeyA) { 1.0 } else { 0.0 };
		self.amount_right = if enabled && input.is_key_pressed(KeyCode::KeyD) { 1.0 } else { 0.0 };
		self.amount_up =if enabled && input.is_key_pressed(KeyCode::KeyE) { 1.0 } else { 0.0 };
		self.amount_down = if enabled && input.is_key_pressed(KeyCode::KeyQ) { 1.0 } else { 0.0 };
	}

	pub fn update(&mut self, input: &Input, camera: &mut Camera, dt: f32) {
		self.process_input(input);

		let (yaw_sin, yaw_cos) = camera.yaw.0.sin_cos();
		let forward = Vector3::new(yaw_cos, 0.0, yaw_sin).normalize();
		let right = Vector3::new(-yaw_sin, 0.0, yaw_cos).normalize();
		camera.position += forward * (self.amount_forward - self.amount_backward) * self.speed * dt;
		camera.position += right * (self.amount_right - self.amount_left) * self.speed * dt;
		camera.position.y += (self.amount_up - self.amount_down) * self.speed * dt;

		camera.yaw += Rad(self.rotate_horizontal) * self.sensitivity * dt;
		camera.pitch += Rad(-self.rotate_vertical) * self.sensitivity * dt;
		self.rotate_horizontal = 0.0;
		self.rotate_vertical = 0.0;

		if camera.pitch < -Rad(SAFE_FRAC_PI_2) {
			camera.pitch = -Rad(SAFE_FRAC_PI_2);
		} else if camera.pitch > Rad(SAFE_FRAC_PI_2) {
			camera.pitch = Rad(SAFE_FRAC_PI_2);
		}
	}
}
