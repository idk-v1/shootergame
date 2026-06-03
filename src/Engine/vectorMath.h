#ifndef VECTORMATH_H
#define VECTORMATH_H

#include <math.h>

#define VEC3FOP(op) \
Vec3f& operator op=(const Vec3f& right) { \
this->x op= right.x; \
this->y op= right.y; \
this->z op= right.z; \
return *this; } \
Vec3f& operator op=(float right) { \
this->x op= right; \
this->y op= right; \
this->z op= right; \
return *this; } \
Vec3f operator op(const Vec3f& right) { \
Vec3f ret = *this; \
ret op= right; \
return ret; } \
Vec3f operator op(float right) { \
Vec3f ret = *this; \
ret op= right; \
return ret; }

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

	struct Vec3f
	{
		float x, y, z;

		Vec3f() : x(0.f), y(0.f), z(0.f) {}
		Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}
		Vec3f(float fill) : x(fill), y(fill), z(fill) {}

		VEC3FOP(+);
		VEC3FOP(-);
		VEC3FOP(*);
		VEC3FOP(/);
		
		Vec3f normalize()
		{
			float dist = fastSqrt(this->x*this->x + this->y*this->y + this->z*this->z);
			return *this / dist;
		}

		float length()
		{
			return fastSqrt(x * x + y * y + z * z);
		}

		Vec3f cross(const Vec3f& right)
		{
			Vec3f vec = { 0 };
			vec.x = this->y * right.z - this->z * right.y;
			vec.y = this->z * right.x - this->x * right.z;
			vec.z = this->x * right.y - this->y * right.x;
			return vec;
		}

		float dot(const Vec3f& right)
		{
			return this->x * right.x + this->y * right.y + this->z * right.z;
		}
	};

	struct Vec2f
	{
		float u, v;
	};


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