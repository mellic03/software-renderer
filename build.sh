#!/bin/bash
gcc ./src/engine/vector.c ./src/engine/engine.c ./src/main.c -o main -lSDL2 -lcblas -lm -g
