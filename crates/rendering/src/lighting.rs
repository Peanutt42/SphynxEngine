

#[repr(C)]
#[derive(Debug, Copy, Clone, bytemuck::Pod, bytemuck::Zeroable)]
pub struct LightUniform {
	pub position: [f32; 3],
	pub _position_padding: f32,
	pub color: [f32; 3],
	pub _color_padding: f32,
}

impl LightUniform {
	pub fn new(position: [f32; 3], color: [f32; 3]) -> Self {
		Self {
			position,
			_position_padding: 0.0,
			color,
			_color_padding: 0.0,
		}
	}
}
