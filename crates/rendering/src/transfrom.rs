use cgmath::{Matrix4, Quaternion, Vector3, Zero};

#[derive(Copy, Clone)]
pub struct Transform {
	position: Vector3<f32>,
	rotation: Quaternion<f32>,
}

impl Transform {
	pub fn new(position: Vector3<f32>, rotation: Quaternion<f32>) -> Self {
		Self {
			position,
			rotation,
		}
	}

	pub fn model_matrix(&self) -> Matrix4<f32> {
		Matrix4::from_translation(self.position) * Matrix4::from(self.rotation)
	}
}

impl Default for Transform {
	fn default() -> Self {
		Self::new(Vector3::zero(), Quaternion::zero())
	}
}
