use proc_macro::TokenStream;

mod vertex;
mod instance;

#[proc_macro_derive(Vertex)]
pub fn vertex_derive(input: TokenStream) -> TokenStream {
	vertex::vertex_derive_impl(input)
}

#[proc_macro_derive(InstanceData)]
pub fn instance_data_derive(input: TokenStream) -> TokenStream {
	instance::instance_data_derive_impl(input)
}
