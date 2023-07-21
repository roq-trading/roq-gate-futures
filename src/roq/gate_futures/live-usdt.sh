#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="gdb --args"
else
  PREFIX=
fi

NAME="gate-futures"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-gate-futures/$CONFIG.toml"

URI="gateio.ws"

REST_URI="https://api.$URI/api/v4"
WS_URI="wss://fx-ws.$URI/v4/ws/usdt"

$PREFIX ./roq-gate-futures \
  --name "gate-futures" \
  --config_file "$CONFIG_FILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --rest_uri "$REST_URI" \
  --ws_uri "$WS_URI" \
  --api "usdt" \
  $@
