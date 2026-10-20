// engine knows this, no need for many duplicate # Safety docs
#![allow(clippy::missing_safety_doc)]

use crate::{Game, SerializedGame};
use sphynx_logging::error;

#[cfg_attr(feature = "hot-reload", unsafe(no_mangle))]
pub fn sphynx_engine_game_create() -> *mut Game {
	Box::into_raw(Box::new(crate::game_create()))
}

#[cfg_attr(feature = "hot-reload", unsafe(no_mangle))]
pub unsafe fn sphynx_engine_game_destroy(game: *mut Game) {
	if !game.is_null() {
		drop(unsafe { Box::from_raw(game) });
	}
}

#[cfg_attr(feature = "hot-reload", unsafe(no_mangle))]
pub unsafe fn sphynx_engine_game_update(game: *mut Game, dt: f32, frame: u64) {
	if game.is_null() {
		return;
	}

	let game = unsafe { &mut *game };

	crate::game_update(game, dt, frame);
}

#[cfg_attr(feature = "hot-reload", unsafe(no_mangle))]
pub unsafe fn sphynx_engine_game_serialize(game: *const Game) -> SerializedGame {
	if game.is_null() {
		panic!("game is null");
	}
	let game = unsafe { &*game };

	match sphynx_hecs_component::serialization::serialize(&game.world) {
		Ok(world_snapshot) => SerializedGame {
			spawn_counter: game.spawn_counter,
			world_snapshot,
		},
		Err(err) => {
			panic!("[GAME] failed to serialize world before reload: {err}");
		}
	}
}

#[cfg_attr(feature = "hot-reload", unsafe(no_mangle))]
pub fn sphynx_engine_game_deserialize(serialized_game: SerializedGame) -> *mut Game {
	let game =
		match sphynx_hecs_component::serialization::deserialize(serialized_game.world_snapshot) {
			Ok(world) => Game {
				world,
				spawn_counter: serialized_game.spawn_counter,
			},
			Err(e) => {
				error!(
					Game,
					"failed to deserialize game snapshot from before hot reloading, \
					recreating game from scratch: {e}"
				);
				crate::game_create()
			}
		};

	Box::into_raw(Box::new(game))
}

#[cfg_attr(feature = "hot-reload", unsafe(no_mangle))]
pub fn sphynx_engine_game_version() -> u64 {
	crate::VERSION
}
