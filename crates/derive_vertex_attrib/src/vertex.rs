use proc_macro::TokenStream;
use quote::quote;
use syn::DeriveInput;

pub fn vertex_derive_impl(input: TokenStream) -> TokenStream {
	let input = syn::parse_macro_input!(input as DeriveInput);

	let struct_name = &input.ident;

	let expanded = quote! {
		impl Vertex for #struct_name {
			fn desc() -> wgpu::VertexBufferLayout<'static> {
				wgpu::VertexBufferLayout {
					array_stride: std::mem::size_of::<#struct_name>() as wgpu::BufferAddress,
					step_mode: wgpu::VertexStepMode::Vertex,
					attributes: &#struct_name::ATTRIBS,
				}
			}
		}
	};
	expanded.into()
}
