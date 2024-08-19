use cgmath::{Angle, InnerSpace, Matrix4, Point3, Rad, Deg, Vector3, perspective};
use wgpu::SurfaceConfiguration;

pub struct Camera {
	pub position: Point3<f32>,
	pub yaw: Rad<f32>,
	pub pitch: Rad<f32>,
	pub fov: f32,
	pub z_near: f32,
	pub z_far: f32,
}

impl Camera {
	const WORLD_UP: Vector3<f32> = Vector3{ x: 0.0, y: 1.0, z: 0.0 };

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

	pub fn get_projection_view_matrix(&self, aspect: f32) -> Matrix4<f32> {
        let proj = perspective(Deg(self.fov), aspect, self.z_near, self.z_far);

        let view = Matrix4::look_to_rh(self.position, self.get_forward_direction(), self.get_up_direction());

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

	pub fn get_aspect_from_swapchain(swapchain_config: &SurfaceConfiguration) -> f32 {
		Self::get_aspect(swapchain_config.width as f32, swapchain_config.height as f32)
	}

	pub fn get_forward_direction(&self) -> Vector3<f32> {
		let (yaw_sin, yaw_cos) = self.yaw.sin_cos();
		let (pitch_sin, pitch_cos) = self.pitch.sin_cos();

		Vector3::new(
			yaw_cos * pitch_cos,
			pitch_sin,
			yaw_sin * pitch_cos
		)
		.normalize()
	}

	pub fn get_right_direction(&self) -> Vector3<f32> {
		self.get_forward_direction()
			.cross(Self::WORLD_UP)
			.normalize()
	}

	pub fn get_up_direction(&self) -> Vector3<f32> {
		self.get_right_direction()
			.cross(self.get_forward_direction())
			.normalize()
	}
}

impl Default for Camera {
	fn default() -> Self {
		Self::new(
			Point3::<f32>::new(0.0, 0.0, 0.0),
			Deg(-90.0).into(),
			Rad(0.0),
			90.0,
			0.1,
			1000.0
		)
	}
}
