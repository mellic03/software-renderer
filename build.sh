#!/bin/bash
gcc \
./src/engine/vector.c \
./src/engine/engine.c \
./src/engine/camera.c \
./src/main.c \
-o main -lSDL2 -lcblas -lm -O3 -g
