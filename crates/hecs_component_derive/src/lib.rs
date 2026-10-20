use proc_macro::TokenStream;
use quote::quote;
use syn::{DeriveInput, parse_macro_input};

/// simply adds derive for [`Debug`], [`Clone`], [`serde::Serialize`],
/// [`serde::Deserialize`] and [`sphynx_hecs_component::HECSComponent`]. nothing more.
/// the [`sphynx_hecs_component::ComponentType`] registering magic is done in the
/// [`derive_hecs_component`] implementation
#[proc_macro_attribute]
pub fn hecs_component(_attr: TokenStream, item: TokenStream) -> TokenStream {
	let mut expanded: TokenStream = quote! {
		#[derive(
			::std::fmt::Debug,
			::std::clone::Clone,
			::sphynx_hecs_component::serde::Serialize,
			::sphynx_hecs_component::serde::Deserialize,
			::sphynx_hecs_component::HECSComponent,
		)]
		#[serde(crate = "::sphynx_hecs_component::serde")]
	}
	.into();
	expanded.extend(item);
	expanded
}

/// Implements [`HECSComponent`], a virtual trait that does not exist.
/// We just need it in order to trigger this proc-macro that auto-generates some functions for
/// hot-reloading and for realtime reflection
#[proc_macro_derive(HECSComponent)]
pub fn derive_hecs_component(input: TokenStream) -> TokenStream {
	let input = parse_macro_input!(input as DeriveInput);
	let name = &input.ident;

	if !input.generics.params.is_empty() {
		return syn::Error::new_spanned(
			&input.generics,
			"HECSComponent does not support generic components yet",
		)
		.to_compile_error()
		.into();
	}

	quote! {
		const _: () = {
			use ::sphynx_hecs_component::hecs;
			use ::sphynx_hecs_component::serde;
			use ::sphynx_hecs_component::serde_json;

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

			::sphynx_hecs_component::inventory::submit! {
				::sphynx_hecs_component::ComponentType {
					type_name,
					has,
					add_to_batch_type,
					serialize: serialize_column,
					deserialize: deserialize_column,
				}
			}
		};
	}
	.into()
}
