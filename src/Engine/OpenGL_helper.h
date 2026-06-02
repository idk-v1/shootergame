#ifndef OPENGL_HELPER_H
#define OPENGL_HELPER_H

#include <glad/glad.h>

#include <vector>
#include <string>

#include "vectorMath.h"

namespace GLH
{
	struct Vertex
	{
		Vec3f pos;
		Vec2f tex;
		Vec3f norm;
	};

	std::string loadTextFile(const std::string& name);

	GLuint loadShader(const std::string& vert, const std::string& frag);
	GLuint loadShaderSrc(const char* vertSrc, const char* fragSrc);
	void unloadShader(GLuint shader);
	void useShader(GLuint shader);
	extern GLuint activeShader;

	struct OGL_Model
	{
		GLuint vao, vbo;
		size_t size;
	};
	OGL_Model loadModel(const std::string& name, float scale = 1.f);
	OGL_Model loadModel(Vertex* verts, size_t length);
	void unloadModel(OGL_Model& model);
	void drawModel(const OGL_Model& model, const Vec3f& pos = {0.f, 0.f, 0.f},
		const Vec3f& rot = {0.f, 0.f, 0.f}, const Vec3f& scale = { 1.f, 1.f, 1.f });

	GLuint loadTexture(const std::string& name);
	GLuint loadTexture(uint8_t* data, size_t width, size_t height);
	extern GLuint noTexture;
	void loadNoTexture();
	void unloadTexture(GLuint texture);
	void useTexture(GLuint texture, GLuint slot = 0);

	void setViewSize(size_t w, size_t h);
	extern size_t screenW, screenH;

	void clear(float r = 0.f, float g = 0.f, float b = 0.f);

	struct Matrix4
	{
		float m[16] = { 0 };

		inline float& operator()(int row, int col)
		{
			return m[row + col * 4];
		}
		inline float operator()(int row, int col) const
		{
			return m[row + col * 4];
		}

		Matrix4 operator*(const Matrix4& r)
		{
			const Matrix4& l = *this;
			Matrix4 ret;

			for (int row = 0; row < 4; ++row)
			{
				for (int col = 0; col < 4; ++col)
				{
					float sum = 0.f;
					for (int i = 0; i < 4; ++i)
						sum += l(row, i) * r(i, col);
					ret(row, col) = sum;
				}
			}

			return ret;
		}

		Vec3f operator*(const Vec3f& r)
		{
			const Matrix4& l = *this;
			Vec3f ret(0.f);

			ret.x = r.x * (l(0, 0) + l(0, 1) + l(0, 2)) + l(0, 3);
			ret.y = r.y * (l(1, 0) + l(1, 1) + l(1, 2)) + l(1, 3);
			ret.z = r.z * (l(2, 0) + l(2, 1) + l(2, 2)) + l(2, 3);

			return ret;
		}
	};
	void setUniformVec3(GLuint shader, const std::string& name, Vec3f value);
	void setUniformVec3(GLuint shader, const char* name, Vec3f value);
	void setUniformFloat(GLuint shader, const std::string& name, float value);
	void setUniformFloat(GLuint shader, const char* name, float value);
	void setUniformInt(GLuint shader, const std::string& name, int value);
	void setUniformInt(GLuint shader, const char* name, int value);
	void setUniformMat4(GLuint shader, const std::string& name, const Matrix4& value);
	void setUniformMat4(GLuint shader, const char* name, const Matrix4& value);

	struct Light
	{
		Vec3f rgb;
		Vec3f pos;
		Vec3f norm;
		float str;
		float type; // easier if its a float, treat like int
	};
	int addLight(const Light& light);
	int addAmbientLight(Vec3f rgb);
	int addDirectionalLight(Vec3f rgb, Vec3f normal);
	int addPointLight(Vec3f rgb, Vec3f pos, float strength);
	void setLight(const Light& light, int index);
	void removeLight(int index);
	extern int lightCount;
	extern bool lightStates[];

	void updateCamera(GLuint shader, Vec3f pos, Vec3f rot, float fov);
}

#endif