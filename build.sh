#!/bin/bash
gcc ./src/engine/shapes.c ./src/engine/vector.c ./src/engine/engine.c ./src/main.c -o main -lSDL2 -lm -g
