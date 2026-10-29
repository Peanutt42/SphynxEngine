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

	let field_count = match &input.data {
		syn::Data::Struct(data) => data.fields.len(),
		_ => 0,
	};

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
					Some(c) => {
						v.begin_component(type_name(), #field_count);
						::sphynx_hecs_component::reflection::Reflect::accept_fields(&*c, v);
						v.end_component(type_name());
						true
					}
					None => false,
				}
			}

			fn has_entity(world: &hecs::World, e: hecs::Entity) -> bool {
				world.get::<&#name>(e).is_ok()
			}

			fn visit_mut(w: &::hecs::World, e: ::hecs::Entity, v: &mut dyn ::sphynx_hecs_component::reflection::VisitorMut) -> bool {
				match w.get::<&mut #name>(e) {
					Ok(mut c) => {
						v.begin_component(type_name(), #field_count);
						::sphynx_hecs_component::reflection::Reflect::accept_fields_mut(&mut *c, v);
						v.end_component(type_name());
						true
					}
					Err(_) => false,
				}
			}

			fn insert_default(world: &mut hecs::World, e: hecs::Entity) {
				let _ = world.insert_one(e, #name::default());
			}

			fn remove(world: &mut hecs::World, e: hecs::Entity) -> bool {
				world.remove_one::<#name>(e).is_ok()
			}

			::sphynx_hecs_component::inventory::submit! {
				::sphynx_hecs_component::ComponentType {
					type_name,
					has,
					has_entity,
					add_to_batch_type,
					serialize: serialize_column,
					deserialize: deserialize_column,
					visit,
					visit_mut,
					insert_default,
					remove,
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
		syn::Data::Enum(e) => return derive_reflect_enum(name, e),
		syn::Data::Union(_) => {
			return syn::Error::new_spanned(
				&input,
				"#[derive(Reflect)] only supports structs and C-style enums",
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
				v.begin_struct(type_name, #field_count);
				::sphynx_hecs_component::reflection::Reflect::accept_fields(self, v);
				v.end_struct(type_name);
			}
			fn accept_fields(&self, v: &mut dyn ::sphynx_hecs_component::reflection::Visitor) {
				#(#ref_fields)*
			}
			fn accept_mut(&mut self, v: &mut dyn ::sphynx_hecs_component::reflection::VisitorMut) {
				let type_name = ::std::any::type_name::<#name>();
				v.begin_struct(type_name, #field_count);
				::sphynx_hecs_component::reflection::Reflect::accept_fields_mut(self, v);
				v.end_struct(type_name);
			}
			fn accept_fields_mut(&mut self, v: &mut dyn ::sphynx_hecs_component::reflection::VisitorMut) {
				#(#mut_fields)*
			}
		}
	}
	.into()
}

fn derive_reflect_enum(name: &syn::Ident, data: &syn::DataEnum) -> TokenStream {
	let mut variant_names = Vec::new();
	let mut selected_arms = Vec::new();
	let mut set_arms = Vec::new();

	for (index, variant) in data.variants.iter().enumerate() {
		if !matches!(variant.fields, syn::Fields::Unit) {
			return syn::Error::new_spanned(
				variant,
				"#[derive(Reflect)] only supports C-style enums without variant fields",
			)
			.into_compile_error()
			.into();
		}

		let variant_name = variant.ident.unraw().to_string();
		let ident = &variant.ident;
		variant_names.push(variant_name);
		selected_arms.push(quote! { Self::#ident => #index });
		set_arms.push(quote! { #index => *self = Self::#ident });
	}

	let selected = if selected_arms.is_empty() {
		quote! { let selected = 0; }
	} else {
		quote! {
			let selected = match self {
				#(#selected_arms),*
			};
		}
	};

	quote! {
		impl ::sphynx_hecs_component::reflection::Reflect for #name {
			fn accept(&self, v: &mut dyn ::sphynx_hecs_component::reflection::Visitor) {
				#selected
				v.visit_value(::sphynx_hecs_component::reflection::Value::Enum(
					::sphynx_hecs_component::reflection::EnumValue {
						type_name: ::std::any::type_name::<#name>(),
						variants: &[#(#variant_names),*],
						selected,
					},
				));
			}

			fn accept_mut(&mut self, v: &mut dyn ::sphynx_hecs_component::reflection::VisitorMut) {
				#selected
				let mut set = |index: usize| {
					match index {
						#(#set_arms,)*
						_ => {}
					}
				};
				v.visit_value(::sphynx_hecs_component::reflection::ValueMut::Enum(
					::sphynx_hecs_component::reflection::EnumValueMut::new(
						::std::any::type_name::<#name>(),
						&[#(#variant_names),*],
						selected,
						&mut set,
					),
				));
			}
		}
	}
	.into()
}
