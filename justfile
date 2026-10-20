default:
    just --list

run-hot-reloadable:
    parallel --line-buffer --halt now,done=1 ::: \
      "cargo run --features hot-reload --color always" \
      "cargo watch -w game -x 'build -p game --features hot-reload --color always'"

run-hot-reloadable-release:
    parallel --line-buffer --halt now,done=1 ::: \
      "cargo run --release --features hot-reload --color always" \
      "cargo watch -w game -x 'build -p game --release --features hot-reload --color always'"
