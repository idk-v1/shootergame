#include "Entity.h"

void GLH::Entity::updatePhysics(float friction)
{
	pos += vel;
	vel /= friction;
}
