#![allow(clippy::not_unsafe_ptr_arg_deref)]

pub mod hot_reload_api;

use hecs::Entity;
use serde::{Deserialize, Serialize};
use sphynx_hecs_component::{Reflect, hecs_component};

/// Just to verify if hot-reload actually happend
pub const VERSION: u64 = 1;

pub struct Game {
	pub world: hecs::World,
	pub spawn_counter: u32,
}

pub struct SerializedGame {
	pub spawn_counter: u32,
	pub world_snapshot: sphynx_hecs_component::serialization::Snapshot,
}

#[derive(Debug, Default, Clone, Serialize, Deserialize, Reflect)]
pub struct Complex {
	field0: u64,
	string: String,
	tuple: (i32, f32, u64),
	fixed_array: [i32; 3],
	dynamic_array: Vec<String>,
	optional: Option<i32>,
	my_enum: MyEnum,
}

#[derive(Debug, Default, Clone, Serialize, Deserialize, Reflect)]
pub enum MyEnum {
	#[default]
	Variant1,
	Variant2,
}

#[hecs_component]
pub struct ComplexComponent {
	complex: Complex,
}

#[hecs_component]
pub struct Position {
	pub x: f32,
	pub y: f32,
}

#[hecs_component]
pub struct Velocity {
	pub dx: f32,
	pub dy: f32,
}

#[hecs_component]
pub struct Health {
	pub hp: f32,
}

#[hecs_component]
pub struct Name {
	pub name: String,
}

#[hecs_component]
pub struct Mana {
	pub amount: u32,
}

pub fn game_create() -> Game {
	let mut world = hecs::World::new();

	world.spawn((
		Name {
			name: "Sample entity to test out entity inspector".to_string(),
		},
		Position { x: 0.0, y: 0.0 },
		Mana { amount: 10 },
		ComplexComponent {
			complex: Complex {
				field0: 0,
				string: "This is a string".to_string(),
				tuple: (0, 0.0, 0),
				fixed_array: [0, 0, 0],
				dynamic_array: vec!["test".to_string()],
				optional: None,
				my_enum: MyEnum::Variant1,
			},
		},
	));

	for i in 0..1_000 {
		world.spawn((
			Position {
				x: i as f32 * 10.0,
				y: 0.0,
			},
			Health { hp: 100.0 },
			Name {
				name: format!("unit-{i}"),
			},
		));
	}

	Game {
		world,
		spawn_counter: 0,
	}
}

pub fn game_update(game: &mut Game, dt: f32, frame: u64) {
	if frame % 10 == 0 {
		spawn(game);
	}

	movement_system(game, dt);

	damage_system(game, dt);

	if frame % 100 == 0 {
		print_summary(game, frame);
	}
}

#[allow(unused)]
fn spawn(game: &mut Game) {
	game.spawn_counter += 1;
	let n = game.spawn_counter;
	game.world.spawn((
		Position { x: 0.0, y: 0.0 },
		Velocity { dx: 1.0, dy: 0.5 },
		Health { hp: 100.0 },
		Name {
			name: format!("bolt-{n}"),
		},
		Mana { amount: 5 },
	));
}

fn movement_system(game: &mut Game, dt: f32) {
	for (pos, vel) in game.world.query_mut::<(&mut Position, &Velocity)>() {
		pos.x += vel.dx * dt;
		pos.y += vel.dy * dt;
	}
}

fn damage_system(game: &mut Game, dt: f32) {
	let mut dead = Vec::new();
	for (e, health) in game.world.query_mut::<(Entity, &mut Health)>() {
		health.hp -= 5.0 * dt;
		if health.hp <= 0.0 {
			dead.push(e);
		}
	}
	for (e, pos) in game.world.query_mut::<(Entity, &Position)>() {
		if pos.x > 1.0 {
			dead.push(e);
		}
	}
	for e in dead {
		let _ = game.world.despawn(e);
	}
}

fn print_summary(game: &Game, frame: u64) {
	println!(
		"\n === Frame: {frame}, Spawn counter: {}, Entity count: {}, Version: {VERSION} ===",
		game.spawn_counter,
		game.world.len()
	);
}
