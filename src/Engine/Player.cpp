#include "Player.h"
#include <SDL3/SDL.h>
#include "vectorMath.h"

void playerController(GLH::Entity& player, float lookSpeed, float& fov, SDL_Window* window)
{
	const bool* keys = SDL_GetKeyboardState(NULL);

	float ry = GLH::toRad(player.rot.y);

	float moveSpeed = player.moveSpeed;
	if (keys[SDL_SCANCODE_LCTRL])
		moveSpeed *= 5.f;

	GLH::Vec3f acc = { 0 };
	if (keys[SDL_SCANCODE_W])
	{
		acc.z -= cosf(ry) * moveSpeed;
		acc.x -= sinf(ry) * moveSpeed;
	}
	if (keys[SDL_SCANCODE_S])
	{
		acc.z += cosf(ry) * moveSpeed;
		acc.x += sinf(ry) * moveSpeed;
	}
	if (keys[SDL_SCANCODE_A])
	{
		acc.z += sinf(ry) * moveSpeed;
		acc.x -= cosf(ry) * moveSpeed;
	}
	if (keys[SDL_SCANCODE_D])
	{
		acc.z -= sinf(ry) * moveSpeed;
		acc.x += cosf(ry) * moveSpeed;
	}
	if (keys[SDL_SCANCODE_SPACE])
	{
		acc.y += moveSpeed;
	}
	if (keys[SDL_SCANCODE_LSHIFT])
	{
		acc.y -= moveSpeed;
	}
	player.vel += acc;

	float mouseX = 0, mouseY = 0;
	SDL_GetRelativeMouseState(&mouseX, &mouseY);
	int w = 0, h = 0;
	SDL_GetWindowSize(window, &w, &h);
	SDL_WarpMouseInWindow(window, w / 2.f, h / 2.f);
	GLH::Vec3f rotAcc = { -mouseY, -mouseX, 0 };
	player.rotVel += rotAcc * lookSpeed;
	player.rot += player.rotVel;
	player.rotVel *= 0.3f;

	player.rot.x = GLH::clampf(-90.f, player.rot.x, 90.f);
	if (player.rot.y < 0.f)
		player.rot.y = 360.f - fmodf(-player.rot.y, 360.f);
	if (player.rot.y >= 360.f)
		player.rot.y = fmodf(player.rot.y, 360.f);

	if (keys[SDL_SCANCODE_RSHIFT])
		fov = fmaxf(fov - 2.f, 10.f);
	else
		fov = fminf(fov + 2.f, 90.f);
}