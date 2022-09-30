#!/bin/bash
gcc \
./src/engine/math/vector.c \
./src/engine/graphics/engine.c \
./src/engine/graphics/camera.c \
./src/engine/fileio/objloader.c \
./src/main.c \
-o main -lSDL2 -mavx -lm -O3 -g \
