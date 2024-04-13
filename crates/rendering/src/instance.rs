use cgmath::Matrix4;
use wgpu::util::DeviceExt;

pub trait InstanceData : bytemuck::Pod + bytemuck::Zeroable {
	fn desc() -> wgpu::VertexBufferLayout<'static>;
}

#[repr(C)]
#[derive(Copy, Clone, bytemuck::Pod, bytemuck::Zeroable)]
pub struct Model_InstanceData {
    pub model: [[f32; 4]; 4],
}

impl Model_InstanceData {
	pub fn new(model: Matrix4<f32>) -> Self {
		Self {
			model: (model).into(),
		}
	}
}

impl InstanceData for Model_InstanceData {
	fn desc() -> wgpu::VertexBufferLayout<'static> {
        use std::mem;
        wgpu::VertexBufferLayout {
            array_stride: mem::size_of::<Self>() as wgpu::BufferAddress,
            // We need to switch from using a step mode of Vertex to Instance
            // This means that our shaders will only change to use the next
            // instance when the shader starts processing a new instance
            step_mode: wgpu::VertexStepMode::Instance,
            attributes: &[
                // A mat4 takes up 4 vertex slots as it is technically 4 vec4s. We need to define a slot
                // for each vec4. We'll have to reassemble the mat4 in the shader.
                wgpu::VertexAttribute {
                    offset: 0,
                    // While our vertex shader only uses locations 0, and 1 now, in later tutorials, we'll
                    // be using 2, 3, and 4, for Vertex. We'll start at slot 5, not conflict with them later
                    shader_location: 5,
                    format: wgpu::VertexFormat::Float32x4,
                },
                wgpu::VertexAttribute {
                    offset: mem::size_of::<[f32; 4]>() as wgpu::BufferAddress,
                    shader_location: 6,
                    format: wgpu::VertexFormat::Float32x4,
                },
                wgpu::VertexAttribute {
                    offset: mem::size_of::<[f32; 8]>() as wgpu::BufferAddress,
                    shader_location: 7,
                    format: wgpu::VertexFormat::Float32x4,
                },
                wgpu::VertexAttribute {
                    offset: mem::size_of::<[f32; 12]>() as wgpu::BufferAddress,
                    shader_location: 8,
                    format: wgpu::VertexFormat::Float32x4,
                },
            ],
        }
    }
}


pub struct InstanceBuffer<I: InstanceData> {
	pub instances: Vec<I>,
	pub buffer: wgpu::Buffer,
}

impl<I: InstanceData> InstanceBuffer<I> {
	pub fn new(instances: Vec<I>, device: &wgpu::Device) -> Self {
		let buffer = device.create_buffer_init(
			&wgpu::util::BufferInitDescriptor {
				label: Some("Instance Buffer"),
				contents: bytemuck::cast_slice(&instances),
				usage: wgpu::BufferUsages::VERTEX | wgpu::BufferUsages::COPY_DST,
			}
		);
		
		Self {
			instances,
			buffer,
		}
	}

	pub fn update(&self, queue: &wgpu::Queue) {
		queue.write_buffer(&self.buffer, 0, bytemuck::cast_slice(&self.instances));
	}
}