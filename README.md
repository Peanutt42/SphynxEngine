# SphynxEngine
 
#### Available commands for fast development (justfile)

```bash
$ just --list
Available recipes:
    build-web *extra_args       # Build the web crate bundle into web/dist
    check-dev *extra_cargo_args # Runs clippy on the entire project using the fast dev cargo configuration (mold linker + cranelift codegen-backend)
    check-web *extra_args       # Run clippy on the entire project using the wasm target
    default                     # List all commands available
    run-dev extra_cargo_args="" # Runs the game using the fast dev cargo configuration (mold linker + cranelift codegen-backend)
    # Runs the game/engine with hot-reload enabled, will rebuild the game lib when change is detected
    run-dev-hotreloadable cargo_args="" engine_only_cargo_args=""
    serve-web *extra_args       # Serve web on http://localhost:8080
```

> [!NOTE]
> When running with the `hot-reload` feature, make sure you have ran `cargo b -p game --features hot-reload` before
> (the `libgame.so` needed by the `hot-reload` feature is only build when explicitly requesting it)
