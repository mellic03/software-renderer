#!/bin/bash
gcc \
./src/engine/vector.c \
./src/engine/engine.c \
./src/engine/camera.c \
./src/main.c \
-o main -lSDL2 -lcblas -lm -O3 -g \
-Werror -Wextra -pedantic -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option
