use proc_macro::TokenStream;
use quote::quote;
use syn::{DeriveInput, ext::IdentExt, parse_macro_input};

/// simply adds derive for [`Debug`], [`Clone`], [`serde::Serialize`],
/// [`serde::Deserialize`], [`sphynx_hecs_component::reflection::Reflect`] and [`sphynx_hecs_component::HECSComponent`]. nothing more.
/// the [`sphynx_hecs_component::ComponentType`] registering magic is done in the
/// [`derive_hecs_component`] implementation
#[proc_macro_attribute]
pub fn hecs_component(_attr: TokenStream, item: TokenStream) -> TokenStream {
	let mut expanded: TokenStream = quote! {
		#[derive(
			::std::fmt::Debug,
			::std::clone::Clone,
			::std::default::Default,
			::sphynx_hecs_component::serde::Serialize,
			::sphynx_hecs_component::serde::Deserialize,
			::sphynx_hecs_component::Reflect,
			::sphynx_hecs_component::HECSComponent,
		)]
		#[serde(crate = "::sphynx_hecs_component::serde")]
	}
	.into();
	expanded.extend(item);
	expanded
}

/// Implements [`HECSComponent`] and [`sphynx_hecs_component::reflection::Reflect`], a virtual trait that does not exist.
/// We just need it in order to trigger this proc-macro that auto-generates some functions for
/// hot-reloading and for realtime reflection
#[proc_macro_derive(HECSComponent)]
pub fn derive_hecs_component(input: TokenStream) -> TokenStream {
	let input = parse_macro_input!(input as DeriveInput);
	let name = &input.ident;

	if !input.generics.params.is_empty() {
		return syn::Error::new_spanned(
			&input.generics,
			"HECSComponent does not support generic components",
		)
		.to_compile_error()
		.into();
	}

	quote! {
		const _: () = {
			use ::sphynx_hecs_component::{hecs, serde, serde_json, reflection::Reflect};

			fn type_name() -> &'static str {
				::std::any::type_name::<#name>()
			}

			fn has(archetype: &hecs::Archetype) -> bool {
				archetype.has::<#name>()
			}

			fn add_to_batch_type(batch_type: &mut hecs::ColumnBatchType) {
				batch_type.add::<#name>();
			}

			fn serialize_column(
				archetype: &hecs::Archetype,
			) -> serde_json::Result<::std::string::String> {
				let Some(column) = archetype.get::<&#name>() else {
					return ::std::result::Result::Err(serde::ser::Error::custom(
						format!("archetype does not have the {} component type", type_name())
					));
				};
				serde_json::to_string(&*column)
			}

			fn deserialize_column(
				components: &str,
				batch: &mut hecs::ColumnBatchBuilder,
			) -> serde_json::Result<()> {
				let values: ::std::vec::Vec<#name> = serde_json::from_str(components)?;
				let mut writer = batch
					.writer::<#name>()
					.ok_or_else(|| serde::de::Error::custom(
						format!("component type {} was not added to the column batch", type_name())
					))?;
				for value in values {
					let _ = writer.push(value);
				}
				::std::result::Result::Ok(())
			}

			fn visit(e: ::hecs::EntityRef<'_>, v: &mut dyn ::sphynx_hecs_component::reflection::Visitor) -> bool {
				match e.get::<&#name>() {
					Some(c) => { c.accept(v); true }
					None => false,
				}
			}

			fn visit_mut(w: &::hecs::World, e: ::hecs::Entity, v: &mut dyn ::sphynx_hecs_component::reflection::VisitorMut) -> bool {
				match w.get::<&mut #name>(e) {
					Ok(mut c) => { c.accept_mut(v); true }
					Err(_) => false,
				}
			}

			::sphynx_hecs_component::inventory::submit! {
				::sphynx_hecs_component::ComponentType {
					type_name,
					has,
					add_to_batch_type,
					serialize: serialize_column,
					deserialize: deserialize_column,
					visit,
					visit_mut,
				}
			}
		};
	}
	.into()
}

#[proc_macro_derive(Reflect)]
pub fn derive_reflect(input: TokenStream) -> TokenStream {
	let input = parse_macro_input!(input as DeriveInput);
	let name = &input.ident;

	if !input.generics.params.is_empty() {
		return syn::Error::new_spanned(
			&input.generics,
			"Reflect does not support generic components",
		)
		.to_compile_error()
		.into();
	}

	let data = match &input.data {
		syn::Data::Struct(s) => s,
		_ => {
			return syn::Error::new_spanned(
				&input,
				"#[derive(Reflect)] only supports structs for now",
			)
			.into_compile_error()
			.into();
		}
	};

	let mut ref_fields = Vec::new();
	let mut mut_fields = Vec::new();

	for (i, field) in data.fields.iter().enumerate() {
		// `self.foo` or `self.0`
		let member = match &field.ident {
			Some(id) => syn::Member::Named(id.clone()),
			None => syn::Member::Unnamed(syn::Index::from(i)),
		};

		// Display name: field ident (without `r#`) > tuple index
		let field_name = match &field.ident {
			Some(id) => id.unraw().to_string(),
			None => i.to_string(),
		};

		ref_fields.push(quote! {
			v.begin_field(#field_name);
			::sphynx_hecs_component::reflection::Reflect::accept(&self.#member, v);
			v.end_field(#field_name);
		});
		mut_fields.push(quote! {
			v.begin_field(#field_name);
			::sphynx_hecs_component::reflection::Reflect::accept_mut(&mut self.#member, v);
			v.end_field(#field_name);
		});
	}

	let field_count = data.fields.len();

	quote! {
		impl ::sphynx_hecs_component::reflection::Reflect for #name {
			fn accept(&self, v: &mut dyn ::sphynx_hecs_component::reflection::Visitor) {
				let type_name = ::std::any::type_name::<#name>();
				v.begin_component(type_name, #field_count);
				#(#ref_fields)*
				v.end_component(type_name);
			}
			fn accept_mut(&mut self, v: &mut dyn ::sphynx_hecs_component::reflection::VisitorMut) {
				let type_name = ::std::any::type_name::<#name>();
				v.begin_component(type_name, #field_count);
				#(#mut_fields)*
				v.end_component(type_name);
			}
		}
	}
	.into()
}
