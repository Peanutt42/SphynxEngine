use hecs::World;
use sphynx_hecs_component::{
	hecs_component,
	serialization::{ComponentColumnRecord, deserialize, serialize},
};

#[hecs_component]
#[derive(PartialEq)]
struct Position {
	x: f32,
	y: f32,
}

#[hecs_component]
#[derive(PartialEq)]
struct Health {
	hp: f32,
}

/// simulates a component that gained a field after the snapshot was taken:
/// the new field needs `#[serde(default)]` to tolerate old snapshots.
#[hecs_component]
#[derive(PartialEq)]
struct RegeneratingHealth {
	hp: f32,
	#[serde(default)]
	regen: f32,
}

#[test]
fn entity_and_components_preservation() {
	let mut world = World::new();
	let full = world.spawn((Position { x: 1.0, y: 2.0 }, Health { hp: 100.0 }));
	let partial = world.spawn((Position { x: 3.0, y: 4.0 },));
	let empty = world.spawn(());

	let snapshot = serialize(&world).unwrap();
	let restored = deserialize(snapshot).unwrap();

	assert_eq!(restored.len(), 3);

	assert_eq!(
		*restored.get::<&Position>(full).unwrap(),
		Position { x: 1.0, y: 2.0 }
	);
	assert_eq!(
		*restored.get::<&Health>(full).unwrap(),
		Health { hp: 100.0 }
	);
	assert_eq!(
		*restored.get::<&Position>(partial).unwrap(),
		Position { x: 3.0, y: 4.0 }
	);
	assert!(restored.get::<&Health>(partial).is_err());
	assert!(restored.get::<&Position>(empty).is_err());
	assert!(restored.get::<&Health>(empty).is_err());
}

#[test]
fn unknown_component_columns_are_skipped() {
	let mut world = World::new();
	let e = world.spawn((Position { x: 1.0, y: 2.0 },));

	let mut snapshot = serialize(&world).unwrap();
	// simulate a snapshot that contains a component type this build doesn't know
	snapshot
		.archetypes
		.get_mut(0)
		.unwrap()
		.columns
		.push(ComponentColumnRecord {
			component_type_name: "future::GhostComponent".to_string(),
			serialized_components: serde_json::json!([{"whatever": true}]).to_string(),
		});

	let restored = deserialize(snapshot).unwrap();
	assert_eq!(
		*restored.get::<&Position>(e).unwrap(),
		Position { x: 1.0, y: 2.0 }
	);
}

#[test]
fn unknown_component_fields_are_ignored() {
	let mut world = World::new();
	let e = world.spawn((Position { x: 1.0, y: 2.0 },));

	let mut snapshot = serialize(&world).unwrap();
	// simulate a snapshot taken before `z` was added to `Position`
	{
		let column = &mut snapshot.archetypes[0].columns[0];
		let mut components: serde_json::Value =
			serde_json::from_str(&column.serialized_components).unwrap();
		components[0]["z"] = serde_json::json!(13.37);
		column.serialized_components = components.to_string();
	}

	let restored = deserialize(snapshot).unwrap();
	assert_eq!(
		*restored.get::<&Position>(e).unwrap(),
		Position { x: 1.0, y: 2.0 }
	);
}

#[test]
fn fields_with_default_tolerate_missing_keys_in_old_snapshots() {
	let mut world = World::new();
	let e = world.spawn((RegeneratingHealth {
		hp: 10.0,
		regen: 1.0,
	},));

	let mut snapshot = serialize(&world).unwrap();
	// simulate an old snapshot taken before `regen` existed
	{
		let column = &mut snapshot.archetypes[0].columns[0];
		let mut components: serde_json::Value =
			serde_json::from_str(&column.serialized_components).unwrap();
		components[0].as_object_mut().unwrap().remove("regen");
		column.serialized_components = components.to_string();
	}

	let restored = deserialize(snapshot).unwrap();
	assert_eq!(
		*restored.get::<&RegeneratingHealth>(e).unwrap(),
		RegeneratingHealth {
			hp: 10.0,
			regen: 0.0
		}
	);
}
