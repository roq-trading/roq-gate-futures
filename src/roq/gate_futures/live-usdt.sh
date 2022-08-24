#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="gdb --args"
else
	PREFIX=
fi

NAME="gate-futures"

CONFIG_FILE="$CWD/config/$NAME.toml"

URI="gateio.ws"

REST_URI="https://api.$URI/api/v4"
WS_URI="wss://fx-ws.$URI/v4/ws/usdt"

$PREFIX ./roq-gate-futures \
	--name "gate-futures" \
	--config_file "$CONFIG_FILE" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --metrics_listen_address "$HOME/run/${NAME}_metrics.sock" \
	--rest_uri "$REST_URI" \
	--ws_uri "$WS_URI" \
	--api "usdt" \
	$@
