#include "Player.h"
#include <SDL3/SDL.h>
#include "vectorMath.h"

void playerController(GLH::Entity& player, float lookSpeed, float& fov)
{
	const bool* keys = SDL_GetKeyboardState(NULL);

	float ry = GLH::toRad(player.rot.y);

	float moveSpeed = player.moveSpeed;
	if (keys[SDL_SCANCODE_LCTRL])
	{
		moveSpeed *= 5.f;
	}

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

	GLH::Vec3f rotAcc = { 0 };
	if (keys[SDL_SCANCODE_RIGHT])
	{
		rotAcc.y -= 1.f;
	}
	if (keys[SDL_SCANCODE_LEFT])
	{
		rotAcc.y += 1.f;
	}
	if (keys[SDL_SCANCODE_DOWN])
	{
		rotAcc.x -= 1.f;
	}
	if (keys[SDL_SCANCODE_UP])
	{
		rotAcc.x += 1.f;
	}
	rotAcc = rotAcc.normalize() * lookSpeed;
	player.rot += rotAcc;

	player.rot.x = GLH::clampf(-90.f, player.rot.x, 90.f);
	if (player.rot.y < 0.f)
		player.rot.y = 360.f - fmodf(-player.rot.y, 360.f);
	if (player.rot.y >= 360.f)
		player.rot.y = fmodf(player.rot.y, 360.f);

	if (keys[SDL_SCANCODE_RSHIFT])
	{
		fov = fmaxf(fov - 2.f, 10.f);
	}
	else
	{
		fov = fminf(fov + 2.f, 90.f);
	}
}