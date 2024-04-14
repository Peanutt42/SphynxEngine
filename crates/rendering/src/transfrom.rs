use cgmath::{Vector3, Matrix4};

pub struct Transform {
	position: Vector3<f32>,
}

impl Transform {
	pub fn new(position: Vector3<f32>) -> Self {
		Self {
			position,
		}
	}

	pub fn model_matrix(&self) -> Matrix4<f32> {
		Matrix4::from_translation(self.position)
	}
}
