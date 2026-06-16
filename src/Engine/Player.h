#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"
#include <SDL3/SDL.h>

void playerController(GLH::Entity& player, float lookSpeed, float& fov, SDL_Window* window);

#endif