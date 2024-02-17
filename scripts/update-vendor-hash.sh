#!/bin/sh
mkdir -p .tmp/vendor
go mod vendor -o .tmp/vendor
go run tailscale.com/cmd/nardump -sri .tmp/vendor > go.mod.sri
rm -rf .tmp/vendor

