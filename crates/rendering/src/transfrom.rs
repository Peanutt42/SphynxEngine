use cgmath::{Matrix3, Matrix4, Quaternion, Vector3, Zero};

use crate::instance_data::Model_InstanceData;

#[derive(Copy, Clone)]
pub struct Transform {
	position: Vector3<f32>,
	rotation: Quaternion<f32>,
	scale: Vector3<f32>,
}

impl Transform {
	pub fn new(position: Vector3<f32>, rotation: Quaternion<f32>, scale: Vector3<f32>) -> Self {
		Self {
			position,
			rotation,
			scale,
		}
	}

	pub fn to_model_instance_data(&self) -> Model_InstanceData {
		let rotation = Matrix3::from(self.rotation);
		let model = Matrix4::from_translation(self.position) * Matrix4::from(rotation) * Matrix4::from_nonuniform_scale(self.scale.x, self.scale.y, self.scale.z);
		Model_InstanceData::new(
			model,
			rotation
		)
	}
}

impl Default for Transform {
	fn default() -> Self {
		Self::new(
			Vector3::zero(),
			Quaternion::zero(),
			Vector3::new(1.0, 1.0, 1.0)
		)
	}
}
