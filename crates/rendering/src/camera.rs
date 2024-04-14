use cgmath::{InnerSpace, Matrix4, Point3, Rad, Vector3};

pub struct Camera {
	pub position: Point3<f32>,
	pub yaw: Rad<f32>,
	pub pitch: Rad<f32>,
	pub fov: f32,
	pub z_near: f32,
	pub z_far: f32,
}

impl Camera {
	pub fn new(position: Point3<f32>, yaw: Rad<f32>, pitch: Rad<f32>, fov: f32, z_near: f32, z_far: f32) -> Self {
		Self {
			position,
			yaw,
			pitch,
			fov,
			z_near,
			z_far,
		}
	}

	pub fn get_projection_view_matrix(&self, aspect: f32) -> cgmath::Matrix4<f32> {
		let (sin_pitch, cos_pitch) = self.pitch.0.sin_cos();
        let (sin_yaw, cos_yaw) = self.yaw.0.sin_cos();

        let view = Matrix4::look_to_rh(self.position, Vector3::new(cos_pitch * cos_yaw, sin_pitch, cos_pitch * sin_yaw).normalize(), Vector3::unit_y());

        const OPENGL_TO_WGPU_MATRIX: Matrix4<f32> = Matrix4::new(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 0.5, 0.5,
            0.0, 0.0, 0.0, 1.0,
        );

        let proj = OPENGL_TO_WGPU_MATRIX * cgmath::perspective(cgmath::Deg(self.fov), aspect, self.z_near, self.z_far);

		proj * view
	}

	/// returns the aspect, if 0.0 or NaN, it returns 16.0 / 9.0 for simplicity (you won't see the result anyways, just don't panic on math)
	pub fn get_aspect(width: f32, height: f32) -> f32 {
		let aspect = width / height;
		if aspect.is_nan() || aspect == 0.0 {
			16.0 / 9.0
		}
		else {
			aspect
		}
	}

	pub fn get_aspect_from_swapchain(swapchain_config: &wgpu::SurfaceConfiguration) -> f32 {
		Self::get_aspect(swapchain_config.width as f32, swapchain_config.height as f32)
	}
}

impl Default for Camera {
	fn default() -> Self {
		Self::new(
			Point3::<f32>::new(0.0, 0.0, 0.0),
			cgmath::Rad(0.0),
			cgmath::Rad(0.0),
			90.0,
			0.1,
			1000.0
		)
	}
}
