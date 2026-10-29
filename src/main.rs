use sphynx_core::{EngineConfig, GameApi};

use game::{Game, SerializedGame};

fn main() -> anyhow::Result<()> {
	#[cfg(feature = "hot-reload")]
	let observer = game_lib::subscribe();

	let game_api = GameApi::<Game, SerializedGame> {
		create: game_lib::sphynx_engine_game_create,
		destroy: game_lib::sphynx_engine_game_destroy,
		update: game_lib::sphynx_engine_game_update,
		serialize: game_lib::sphynx_engine_game_serialize,
		deserialize: game_lib::sphynx_engine_game_deserialize,
		version: game_lib::sphynx_engine_game_version,
		get_hecs_world: game_lib::sphynx_engine_game_get_hecs_world,
		get_component_types: game_lib::sphynx_engine_game_get_component_types,
	};

	let fullscreen = false;
	// so that my laptop does not burn while debugging, maybe should add fps-limiter next
	let vsync = cfg!(debug_assertions);

	EngineConfig::new(fullscreen, vsync).run(
		game_api,
		#[cfg(feature = "hot-reload")]
		observer,
	)
}

/// either this hot-reloadable implementation is "game_lib"
#[cfg(feature = "hot-reload")]
#[hot_lib_reloader::hot_module(dylib = "game", file_watch_debounce = 50)]
mod game_lib {
	// needed for the proc-macro magic
	use game::{Game, SerializedGame};

	hot_functions_from_file!("game/src/hot_reload_api.rs");

	#[lib_change_subscription]
	pub fn subscribe() -> hot_lib_reloader::LibReloadObserver {}
}

/// or this statically linked implementation is "game_lib"
#[cfg(not(feature = "hot-reload"))]
use game::hot_reload_api as game_lib;
