#!/bin/sh

export LD_PRELOAD="../build/libhook.so"

exe=$1
shift

exec "$exe" "$@"
