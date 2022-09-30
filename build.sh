#!/bin/bash
gcc \
./src/engine/GraphicsEngine/graphics.c \
./src/engine/GraphicsEngine/objloader.c \
./src/engine/GraphicsEngine/camera.c \
./src/engine/math/vector.c \
./src/engine/PhysicsEngine/physics.c \
./src/engine/GameEngine/gameengine.c \
./src/main.c \
-o main -lSDL2 -mavx -lm -O3 -g \
