#!/bin/sh
# Compile every examples/*.ml to C, build it, and run it with sample input.
set -e
mkdir -p build
for src in examples/*.ml; do
    name=$(basename "$src" .ml)
    dir="build/$name"
    mkdir -p "$dir"
    (cd "$dir" && ../../micro-ml "../../$src" "$name.c" && ${CC:-cc} -Wall "$name.c" -o "$name")
    input="examples/$name.in"
    printf '%s: ' "$name"
    if [ -f "$input" ]; then "$dir/$name" < "$input" | sed 's/Enter Number: //g'; else "$dir/$name"; fi
done
