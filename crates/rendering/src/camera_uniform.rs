use crate::Camera;


#[repr(C)]
#[derive(Debug, Copy, Clone, bytemuck::Pod, bytemuck::Zeroable)]
pub struct CameraUniform {
	proj_view: [[f32; 4]; 4],
}

impl CameraUniform {
	pub fn new(proj_view: cgmath::Matrix4<f32>) -> Self {
		Self {
			proj_view: proj_view.into(),
		}
	}

	pub fn from_camera(camera: &Camera, aspect: f32) -> Self {
		Self::new(camera.get_projection_view_matrix(aspect))
	}
}
