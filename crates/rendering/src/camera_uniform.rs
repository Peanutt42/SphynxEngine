use cgmath::Matrix4;
use crate::Camera;

#[repr(C)]
#[derive(Debug, Copy, Clone, bytemuck::Pod, bytemuck::Zeroable)]
pub struct CameraUniform {
	proj_view: [[f32; 4]; 4],
	position: [f32; 3],
	_position_padding: f32,
}

impl CameraUniform {
	pub fn new(position: [f32; 3], proj_view: Matrix4<f32>) -> Self {
		Self {
			proj_view: proj_view.into(),
			position,
			_position_padding: 0.0,
		}
	}

	pub fn from_camera(camera: &Camera, aspect: f32) -> Self {
		Self::new(
			camera.position.into(),
			camera.get_projection_view_matrix(aspect)
		)
	}
}
