#!/usr/bin/env bash
set -e

NAME="${1:-paint}"
OUT="${NAME}.bin"

gcc $(pkg-config --cflags gtk4) main.c canvas.c ui.c -o "$OUT" $(pkg-config --libs gtk4) -lm
echo "Compilado com sucesso: ./$OUT"