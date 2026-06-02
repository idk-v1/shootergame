#ifndef ENTITY_H
#define ENTITY_H

#include "vectorMath.h"
#include "OpenGL_helper.h"

namespace GLH
{

	struct Entity
	{
		Vec3f pos;
		Vec3f vel;
		Vec3f rot;
		float moveSpeed;

		Entity() {}

		Entity(const Vec3f& pos, const Vec3f& rot = {0.f, 0.f, 0.f}, float moveSpeed = 0.f) : 
			pos(pos), rot(rot), moveSpeed(moveSpeed), vel({0.f, 0.f, 0.f})
		{
		}

		void updatePhysics(float friction);
	};
}

#endif