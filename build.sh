#!/bin/bash
gcc \
./sockets/client.c \
\
./src/engine/math/vector.c \
\
./src/engine/GraphicsEngine/graphics.c \
./src/engine/GraphicsEngine/lighting.c \
./src/engine/GraphicsEngine/model.c \
./src/engine/GraphicsEngine/objloader.c \
./src/engine/GraphicsEngine/camera.c \
./src/engine/GraphicsEngine/datastructures/queue.c \
./src/engine/GraphicsEngine/datastructures/bsp.c \
\
./src/engine/PhysicsEngine/physics.c \
./src/engine/PhysicsEngine/collision.c \
\
./src/engine/GameEngine/gameengine.c \
./src/engine/GameEngine/player.c \
./src/engine/GameEngine/input.c \
\
./src/main.c \
-o main -lSDL2 -lcblas -mavx -lm -Ofast -g \

