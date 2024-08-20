use std::hash::{Hash, Hasher};

use wgpu::{VertexAttribute, VertexBufferLayout, vertex_attr_array};
use sphynx_derive_vertex_attrib::Vertex;

pub trait Vertex : bytemuck::Pod + bytemuck::Zeroable {
	fn desc() -> VertexBufferLayout<'static>;
}

#[repr(C)]
#[derive(Copy, Clone, Debug, bytemuck::Pod, bytemuck::Zeroable, Vertex)]
#[allow(non_camel_case_types)]
pub struct PCN_Vertex {
	pub position: [f32; 3],
	pub color: [f32; 3],
	pub normal: [f32; 3],
}

impl PCN_Vertex {
	const ATTRIBS: [VertexAttribute; 3] = vertex_attr_array![
		0 => Float32x3,
		1 => Float32x3,
		2 => Float32x3
	];
}

impl PartialEq for PCN_Vertex {
    fn eq(&self, other: &Self) -> bool {
        self.position == other.position
            && self.color == other.color
            && self.normal == other.normal
    }
}

impl Eq for PCN_Vertex {}

impl Hash for PCN_Vertex {
    fn hash<H: Hasher>(&self, state: &mut H) {
        self.position[0].to_bits().hash(state);
        self.position[1].to_bits().hash(state);
        self.position[2].to_bits().hash(state);
        self.color[0].to_bits().hash(state);
        self.color[1].to_bits().hash(state);
        self.color[2].to_bits().hash(state);
        self.normal[0].to_bits().hash(state);
        self.normal[1].to_bits().hash(state);
        self.normal[2].to_bits().hash(state);
    }
}