default:
    just --list

run-hot-reloadable cargo_args engine_only_cargo_args:
    parallel --line-buffer --halt now,done=1 ::: \
      "cargo run --features hot-reload --color always {{cargo_args}} {{engine_only_cargo_args}}" \
      "cargo watch -w game -x 'build -p game --features hot-reload --color always {{cargo_args}}'"
