#ifndef VECTORMATH_H
#define VECTORMATH_H

#include <math.h>
#include <NPJ/Vector.h>

namespace GLH
{
	static const float PI = 3.1415f;

	static float fastSqrt(float x)
	{
		union
		{
			int i;
			float x;
		} u;
		u.x = x;
		u.i = (1 << 29) + (u.i >> 1) - (1 << 22);

		// Two Babylonian Steps (simplified from:)
		// u.x = 0.5f * (u.x + x/u.x);
		// u.x = 0.5f * (u.x + x/u.x);
		u.x = u.x + x / u.x;
		u.x = 0.25f * u.x + x / u.x;

		return u.x;
	}

	static float fastAtan2(float y, float x)
	{
		//http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
		//Volkan SALMA

		const float ONEQTR_PI = PI / 4.f;
		const float THRQTR_PI = 3.f * PI / 4.f;
		float r, angle;
		float abs_y = fabsf(y) + 1e-10f;      // kludge to prevent 0/0 condition
		if (x < 0.f)
		{
			r = (x + abs_y) / (abs_y - x);
			angle = THRQTR_PI;
		}
		else
		{
			r = (x - abs_y) / (x + abs_y);
			angle = ONEQTR_PI;
		}
		angle += (0.1963f * r * r - 0.9817f) * r;
		if (y < 0.f)
			return(-angle);     // negate if in quad III or IV
		else
			return(angle);
	}

	using Vec3f = npj::Vector<float, 3>;
	using Vec2f = npj::Vector<float, 2, npj::VectorSemantic::UV>;


	static float toRad(float deg)
	{
		return deg * PI / 180.f;
	}

	static float toDeg(float rad)
	{
		return rad * 180.f / PI;
	}

	static float wrapDeg(float deg)
	{
		deg = fmodf(deg, 360.f);
		if (deg < 0.f)
			deg += 360.f;
		return deg;
	}

	static float clampf(float min, float value, float max)
	{
		return fmaxf(fminf(value, max), min);
	}

	static float lerp(float x, float y, float mix)
	{
		return x * mix + y * (1.f - mix);
	}
}

#endif