gcc ^
./src/engine/math/vector.c ^
./src/engine/GraphicsEngine/graphics.c ^
./src/engine/GraphicsEngine/model.c ^
./src/engine/GraphicsEngine/objloader.c ^
./src/engine/GraphicsEngine/camera.c ^
./src/engine/PhysicsEngine/physics.c ^
./src/engine/PhysicsEngine/collision.c ^
./src/engine/GameEngine/gameengine.c ^
./src/engine/GameEngine/player.c ^
./src/engine/GameEngine/input.c ^
./src/main.c ^
-o main -L src/lib -I src/include/SDL2 -lSDL2 -mavx -lm -O3 -g ^
