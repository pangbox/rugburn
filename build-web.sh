#!/bin/sh
set -e
mkdir -p ./web/dist
cp ./web/index.html ./web/dist/index.html
cp ./web/main.js ./web/dist/main.js
cp ./web/wasm_exec.js ./web/dist/wasm_exec.js
GOOS=js GOARCH=wasm go build -o ./web/dist/patcher.wasm ./web/patcher
