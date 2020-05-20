#!/usr/bin/env bash

if [ -z "$1" ]; then
    TAG=latest
else
    TAG="$1"
fi

docker build -t registry.thinkit.ir/teletin:$TAG .
