#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="gdb --args"
else
	PREFIX=
fi

NAME="gate-io-testnet"

CONFIG_FILE="$CWD/config/$NAME.toml"

URI="gate-io.com"

REST_URI="https://api-testnet.$URI"
WS_PUBLIC_URI="wss://stream-testnet.$URI/spot/quote/ws/v2"
WS_PRIVATE_URI="wss://stream-testnet.$URI/spot/ws"


$PREFIX ./roq-gate-io \
	--name "gate-io" \
	--config_file "$CONFIG_FILE" \
	--client_listen_address $CWD/$NAME.sock \
	--metrics_listen_address 2345 \
	--rest_uri "$REST_URI" \
	--ws_public_uri "$WS_PUBLIC_URI" \
	--ws_private_uri "$WS_PRIVATE_URI" \
	$@
