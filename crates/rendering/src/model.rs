use std::io::BufRead;
use wgpu::Device;
use wavefront::{Obj, Vertex};
use crate::{vertex::PCN_Vertex, Mesh};

#[macro_export]
macro_rules! include_model {
	($filepath:expr, $device:expr) => {{
		load_obj_model(include_bytes!($filepath) as &[u8], $device)
	}};
}

pub fn load_obj_model<B>(buf_reader: B, device: &Device) -> anyhow::Result<Mesh> where B: BufRead {
	let model = Obj::from_reader(buf_reader)?;

	let mut vertices = Vec::new();
	let mut indices = Vec::new();

	let wavefront_vertex_to_pcn_vertex = |vertex: Vertex| {
		PCN_Vertex {
			position: vertex.position(),
			color: [1.0, 1.0, 1.0],
			normal: vertex.normal().unwrap_or([0.0, 0.0, 0.0])
		}
	};

	for [vertex_a, vertex_b, vertex_c] in model.triangles() {
		vertices.push(wavefront_vertex_to_pcn_vertex(vertex_a));
		vertices.push(wavefront_vertex_to_pcn_vertex(vertex_b));
		vertices.push(wavefront_vertex_to_pcn_vertex(vertex_c));
		indices.push(indices.len() as u32);
		indices.push(indices.len() as u32);
		indices.push(indices.len() as u32);
	}

	Ok(Mesh::with_vertices(&vertices, Some(&indices), device))
}