// lets the generated derive code refer to this crate by name (also inside its own tests)
extern crate self as sphynx_hecs_component;

pub use hecs;
pub use inventory;
pub use serde;
pub use serde_json;
pub use sphynx_hecs_component_derive::{HECSComponent, hecs_component};

pub mod serialization;

/// One registered component type for the column-major snapshot format.
///
/// The function pointers are monomorphized per component type and stay valid
/// for the lifetime of the current dylib image. [`HECSComponent`] emits a
/// `submit!` for each component type.
pub struct ComponentType {
	/// key to identify component type across different builds (hot-reloading!).
	/// set to [`std::any::type_name`] by the proc-macro.
	pub type_name: fn() -> &'static str,
	/// whether [`hecs::Archetype`] stores this component type.
	pub has: fn(archetype: &hecs::Archetype) -> bool,
	/// add this component type to a column batch under construction.
	pub add_to_batch_type: fn(batch_type: &mut hecs::ColumnBatchType),
	/// serialize every value of this component column into a JSON array string.
	pub serialize: fn(archetype: &hecs::Archetype) -> serde_json::Result<String>,
	/// deserialize a JSON array of values and append them to `batch`.
	pub deserialize:
		fn(json: &str, batch_builder: &mut hecs::ColumnBatchBuilder) -> serde_json::Result<()>,
}

inventory::collect!(ComponentType);

/// iterator over every [`ComponentType`] registered in the current dylib.
pub fn registered_component_types() -> impl IntoIterator<Item = &'static ComponentType> {
	inventory::iter::<ComponentType>
}
