#include "OpenGL_helper.h"
#include "ObjReader.h"

#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "builtinModels.h"

//https://www.reddit.com/r/opengl/comments/unc3fy/how_to_programatically_set_the_gpu_to_my_opengl/
#ifdef _WIN32
extern "C"
{
	typedef unsigned long DWORD;
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
	_declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

size_t GLH::screenW = 0, GLH::screenH = 0;

std::string GLH::loadTextFile(const std::string& name)
{
    std::string ret;

    std::ifstream file(name);
    if (file.is_open())
    {
        std::ostringstream oss;
        oss << file.rdbuf();
        ret = oss.str();
        file.close();
    }

    return ret;
}


GLuint GLH::loadShader(const std::string& vert, const std::string& frag)
{
    std::string vertSrc = loadTextFile(vert);
    std::string fragSrc = loadTextFile(frag);
    return loadShaderSrc(vertSrc.data(), fragSrc.data());
}

GLuint GLH::loadShaderSrc(const char* vertSrc, const char* fragSrc)
{
	GLuint shader = 0;

	int status = 0;
	char info[512];

	unsigned vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertSrc, NULL);
	glCompileShader(vertShader);
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		std::cout << "Failed to compile vertex shader\n";
		glGetShaderInfoLog(vertShader, 512, NULL, info);
		std::cout << info << '\n';
	}

	unsigned fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSrc, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		std::cout << "Failed to compile fragment shader\n";
		glGetShaderInfoLog(fragShader, 512, NULL, info);
		std::cout << info << '\n';
	}

	shader = glCreateProgram();
	glAttachShader(shader, vertShader);
	glAttachShader(shader, fragShader);
	glLinkProgram(shader);
	glGetProgramiv(shader, GL_LINK_STATUS, &status);
	if (!status)
	{
		std::cout << "Failed to link shader\n";
		glGetProgramInfoLog(shader, 512, NULL, info);
		std::cout << info << '\n';
	}

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return shader;
}

void GLH::unloadShader(GLuint shader)
{
	glDeleteProgram(shader);
}

void GLH::useShader(GLuint shader)
{
	glUseProgram(shader);
	activeShader = shader;
}
GLuint GLH::activeShader = 0;


GLH::OGL_Model GLH::loadModel(const std::string& name, float scale, GLH::Vec3f offset)
{
	std::vector<GLH::Vertex> data = readObjFile(name);
	if (scale != 1.f)
	{
		std::ofstream file(name.substr(0, name.size() - 4) + "OUT.obj");
		GLH::Vec3f avg;
		for (size_t i = 0; i < data.size(); ++i)
		{
			data[i].pos *= scale;
			data[i].pos -= offset;
			avg += data[i].pos;
			file << data[i].pos.x << ", " << data[i].pos.y << ", " << data[i].pos.z << ",\n";
		}
		avg /= data.size();
		float maxDist = 0.f;
		for (size_t i = 0; i < data.size(); ++i)
			maxDist = fmaxf(maxDist, (data[i].pos - avg).length());
		file << "avg " << avg.x << " " << avg.y << " " << avg.z << " rad " << maxDist;
		file.close();
	}
	return loadModel(data.data(), data.size());
}

GLH::OGL_Model GLH::loadModel(Vertex* verts, size_t length)
{
	OGL_Model model = { 0 };
	model.size = length;

	glGenVertexArrays(1, &model.vao);
	glBindVertexArray(model.vao);

	glGenBuffers(1, &model.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model.vbo);
	glBufferData(GL_ARRAY_BUFFER, length * sizeof(Vertex), verts, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(Vec3f));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(Vec3f)+sizeof(Vec2f)));
	glEnableVertexAttribArray(2);

	for (size_t i = 0; i < length; ++i)
	{
		float vecLen = verts[i].pos.length();
		if (model.boundingRad < vecLen)
			model.boundingRad = vecLen;
	}

	return model;
}

void GLH::unloadModel(OGL_Model& model)
{
	glDeleteVertexArrays(1, &model.vao);
	glDeleteBuffers(1, &model.vbo);
	model.size = 0;
	model.vao = 0;
	model.vbo = 0;
}


static bool sphereCollision(GLH::Vec3f aPos, GLH::Vec3f bPos, float aRad, float bRad)
{
	GLH::Vec3f diff = aPos - bPos;
	float dist = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	return (dist < (aRad + bRad) * (aRad + bRad));
}

void GLH::drawModel(const OGL_Model& model, GLuint texture,
	const Vec3f& pos, const Vec3f& rot, const Vec3f& scale)
{
	Matrix4 mat;
	float rx = toRad(rot.x);
	float ry = toRad(rot.y);
	float rz = toRad(rot.z);

	float ax = scale.x;
	float ay = scale.y;
	float az = scale.z;
	float ox = pos.x;
	float oy = pos.y;
	float oz = pos.z;
	float cx = cosf(rx);
	float cy = cosf(ry);
	float cz = cosf(rz);
	float sx = sinf(rx);
	float sy = sinf(ry);
	float sz = sinf(rz);
	mat(0, 0) = ax*cy*cz;
	mat(0, 1) = ax*cy*sz;
	mat(0, 2) = -ax*sy;
	mat(0, 3) = ox;
	mat(1, 0) = az*sx*sy*cz - ay*cx*sz;
	mat(1, 1) = az*sx*sy*sz + ay*cx*cz;
	mat(1, 2) = az*sx*cy;
	mat(1, 3) = oy;
	mat(2, 0) = az*cx*sy*cz + az*sx*sz;
	mat(2, 1) = az*cx*sy*sz - az*sx*cz;
	mat(2, 2) = az*cx*cy;
	mat(2, 3) = oz;
	mat(3, 3) = 1.f;

	// check if model bounding sphere should be culled

	setUniformMat4(activeShader, "modelMat", mat);
	glBindVertexArray(model.vao);

	GLH::useTexture(texture);


	int lightStates[10] = { 0 }; // ceil(300 / 32) = 10
	for (int i = 0; i < lightCount; ++i)
	{
		if (lights[i].type == 3 || lights[i].type == 4)
		{
			float maxScale = fmaxf(sx, fmaxf(sy, sz));
			if (sphereCollision(pos, lights[i].pos, model.boundingRad * maxScale, lights[i].str))
				lightStates[i >> 5] |= 1 << (i & 31);
		}
		else if (lights[i].type == 0);
		else
			lightStates[i >> 5] |= 1 << (i & 31);
	}

	glUniform1iv(glGetUniformLocation(activeShader, "lightStates"), 10, lightStates);

	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)model.size);
}


GLuint GLH::noTexture = 0;
void GLH::loadNoTexture()
{
	uint32_t pix = 0xFFFFFFFF;
	noTexture = loadTexture((uint8_t*)&pix, 1, 1);
}

GLuint GLH::loadTexture(const std::string& name)
{
	int width, height, comp;
	stbi_set_flip_vertically_on_load(true);
	stbi_uc* data = stbi_load(name.data(), &width, &height, &comp, 4);
	if (data)
	{
		GLuint ret = loadTexture(data, width, height);
		stbi_image_free(data);
		return ret;
	}
	else
	{
		std::cout << "Failed to load image: " << name << '\n';
		return 0;
	}
}

GLuint GLH::loadTexture(uint8_t* data, size_t width, size_t height)
{
	GLuint texture = 0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	return texture;
}

void GLH::unloadTexture(GLuint texture)
{
	glDeleteTextures(1, &texture);
}

void GLH::useTexture(GLuint texture, GLuint slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, texture);
}

void GLH::setViewSize(size_t w, size_t h, OGL_RenderBuffer& renderBuf)
{
	renderBuf.width = w;
	renderBuf.height = h;
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	screenW = w;
	screenH = h;
}

void GLH::clear(float r, float g, float b)
{
	glClearColor(r, g, b, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLH::clearDepth()
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

void GLH::setUniformVec3(GLuint shader, const std::string& name, Vec3f value)
{
	setUniformVec3(shader, name.data(), value);
}

void GLH::setUniformVec3(GLuint shader, const char* name, Vec3f value)
{
	glUniform3f(glGetUniformLocation(shader, name), value.x, value.y, value.z);
}

void GLH::setUniformFloat(GLuint shader, const std::string& name, float value)
{
	setUniformFloat(shader, name.data(), value);
}

void GLH::setUniformFloat(GLuint shader, const char* name, float value)
{
	glUniform1f(glGetUniformLocation(shader, name), value);
}

void GLH::setUniformInt(GLuint shader, const std::string& name, int value)
{
	setUniformInt(shader, name.data(), value);
}

void GLH::setUniformInt(GLuint shader, const char* name, int value)
{
	glUniform1i(glGetUniformLocation(shader, name), value);
}

void GLH::setUniformMat4(GLuint shader, const std::string& name, const Matrix4& value)
{
	setUniformMat4(shader, name.data(), value);
}

void GLH::setUniformMat4(GLuint shader, const char* name, const Matrix4& value)
{
	glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, false, value.m);
}

int GLH::lightCount = 0; // count of light storage in use, not actual count
bool GLH::lightStates[150] = { 0 };
extern GLH::Light GLH::lights[150] = { 0 };


int GLH::addLight(const Light& light)
{
	if (lightCount < 150)
	{
		for (int i = 0; i < lightCount; ++i)
		{
			if (!lightStates[i])
			{
				lightStates[i] = true;
				lights[i] = light;
				std::string name = "lights[" + std::to_string(i) + "]";
				glUniformMatrix4x3fv(glGetUniformLocation(activeShader, name.data()), 1, false, (float*)&light);
				return i;
			}
		}

		lightStates[lightCount] = true;
		lights[lightCount] = light;
		std::string name = "lights[" + std::to_string(lightCount) + "]";
		glUniformMatrix4x3fv(glGetUniformLocation(activeShader, name.data()), 1, false, (float*)&light);
		++lightCount;
		setUniformInt(activeShader, "lightCount", lightCount);
		return lightCount - 1;
	}

	return -1;
}

int GLH::addAmbientLight(Vec3f rgb)
{
	Light light = { 0 };
	light.rgb = rgb;
	light.type = 1.f;
	return addLight(light);
}

int GLH::addDirectionalLight(Vec3f rgb, Vec3f normal, float spread)
{
	Light light = { 0 };
	light.rgb = rgb;
	light.norm = normal;
	light.type = 2.f;
	light.spread = 1.f / spread;
	return addLight(light);
}

int GLH::addPointLight(Vec3f rgb, Vec3f pos, float strength)
{
	Light light = { 0 };
	light.rgb = rgb;
	light.pos = pos;
	light.str = strength;
	light.type = 3.f;
	return addLight(light);
}

int GLH::addDirectionalPointLight(Vec3f rgb, Vec3f pos, Vec3f normal, float strength, float spread)
{
	Light light = { 0 };
	light.rgb = rgb;
	light.pos = pos;
	light.norm = normal;
	light.str = strength;
	light.spread = 1.f / spread;
	light.type = 4.f;
	return addLight(light);
}

void GLH::setLight(const Light& light, int index)
{
	if (index >= 0 && index < 300)
	{
		lightStates[index] = true;
		lights[index] = light;
		std::string name = "lights[" + std::to_string(index) + "]";
		glUniformMatrix4x3fv(glGetUniformLocation(activeShader, name.data()), 1, false, (float*)&light);
	}
}

void GLH::removeLight(int index)
{
	if (index >= 0 && index < 300)
	{
		lightStates[index] = false;
		std::string name = "lights[" + std::to_string(index) + "]";
		lights[index].type = 0;
		glUniformMatrix4x3fv(glGetUniformLocation(activeShader, name.data()), 1, false, (float*)&lights[index]);
	}
}


void GLH::updateCamera(Vec3f pos, Vec3f rot, float fov)
{
	Matrix4 projMat = { 0 };
	Matrix4 viewMat = { 0 };

	float aspect = (float)screenW / (float)screenH;

	float far = 1000.f;
	float near = 0.01f;

	projMat(0, 0) = 1.f / (aspect * tanf(toRad(fov) / 2.f));
	projMat(1, 1) = 1.f / tanf(toRad(fov) / 2.f);
	projMat(2, 2) = (far + near) / (near - far);
	projMat(3, 2) = -1.f;
	projMat(2, 3) = -(2.f * far * near) / (far - near);

	Vec3f dir = Vec3f(
		-sinf(toRad(rot.y)) * cosf(toRad(rot.x)),
		sinf(toRad(rot.x)),
		-cosf(toRad(rot.y)) * cosf(toRad(rot.x))
	).normalize();
	Vec3f right = dir.cross(Vec3f(0.f, 1.f, 0.f)).normalize();
	Vec3f up = right.cross(dir);

	viewMat(0, 0) =  right.x;
	viewMat(0, 1) =  right.y;
	viewMat(0, 2) =  right.z;
	viewMat(1, 0) =  up.x;
	viewMat(1, 1) =  up.y;
	viewMat(1, 2) =  up.z;
	viewMat(2, 0) = -dir.x;
	viewMat(2, 1) = -dir.y;
	viewMat(2, 2) = -dir.z;
	viewMat(0, 3) = -right.dot(pos);
	viewMat(1, 3) = -up.dot(pos);
	viewMat(2, 3) =  dir.dot(pos);
	viewMat(3, 3) =  1.f;

	setUniformVec3(activeShader, "camPos", pos);
	setUniformVec3(activeShader, "camRot", dir);
	setUniformMat4(activeShader, "projMat", projMat);
	setUniformMat4(activeShader, "viewMat", viewMat);
}


GLH::OGL_Model GLH::cubeModel = { 0 };

void GLH::loadCubeModel()
{
	memset(&cubeModel, 0, sizeof(OGL_Model));
	cubeModel.size = sizeof(skyboxVertices) / sizeof(float) / 3;

	glGenVertexArrays(1, &cubeModel.vao);
	glBindVertexArray(cubeModel.vao);

	glGenBuffers(1, &cubeModel.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cubeModel.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), (void*)0);
	glEnableVertexAttribArray(0);
}

GLuint GLH::loadSkybox(const std::string& name)
{
	int width, height, comp;
	stbi_set_flip_vertically_on_load(false);
	stbi_uc* data = stbi_load(name.data(), &width, &height, &comp, 4);
	if (data)
	{
		int cubeSize = height / 3;

		stbi_uc* subData = (stbi_uc*)malloc((size_t)cubeSize * cubeSize * 4);
		if (subData)
		{
			int coords[] =
			{
				2, 1, // +x
				0, 1, // -x
				1, 0, // +y
				1, 2, // -y
				1, 1, // +z
				3, 1, // -z
			};

			GLuint id = 0;
			glGenTextures(1, &id);
			glBindTexture(GL_TEXTURE_CUBE_MAP, id);
			for (int i = 0; i < 6; ++i)
			{
				for (int y = 0; y < cubeSize; ++y)
				{
					int ox = cubeSize * coords[i * 2];
					int oy = width * (coords[i * 2 + 1] * cubeSize + y);
					memcpy(subData + cubeSize * 4 * y, data + (ox + oy) * 4, cubeSize * 4ull);
				}
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, cubeSize, cubeSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, subData);
			}
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			free(subData);
			stbi_image_free(data);
			return id;
		}
		else
			stbi_image_free(data);

	}

	std::cout << "Failed to load image: " << name << '\n';
	return 0;
}


void GLH::drawSkybox(Vec3f rot, float fov, GLuint texture)
{
	Matrix4 projMat = { 0 };
	Matrix4 viewMat = { 0 };

	float aspect = (float)screenW / (float)screenH;

	float far = 1000.f;
	float near = 0.01f;

	projMat(0, 0) = 1.f / (aspect * tanf(toRad(fov) / 2.f));
	projMat(1, 1) = 1.f / tanf(toRad(fov) / 2.f);
	projMat(2, 2) = (far + near) / (near - far);
	projMat(3, 2) = -1.f;
	projMat(2, 3) = -(2.f * far * near) / (far - near);

	Vec3f dir = Vec3f(
		-sinf(toRad(rot.y)) * cosf(toRad(rot.x)),
		sinf(toRad(rot.x)),
		-cosf(toRad(rot.y)) * cosf(toRad(rot.x))
	).normalize();
	Vec3f right = dir.cross(Vec3f(0.f, 1.f, 0.f)).normalize();
	Vec3f up = right.cross(dir);

	Vec3f pos(0.f, 0.f, 0.f);
	viewMat(0, 0) = right.x;
	viewMat(0, 1) = right.y;
	viewMat(0, 2) = right.z;
	viewMat(1, 0) = up.x;
	viewMat(1, 1) = up.y;
	viewMat(1, 2) = up.z;
	viewMat(2, 0) = -dir.x;
	viewMat(2, 1) = -dir.y;
	viewMat(2, 2) = -dir.z;
	viewMat(0, 3) = -right.dot(pos);
	viewMat(1, 3) = -up.dot(pos);
	viewMat(2, 3) = dir.dot(pos);
	viewMat(3, 3) = 1.f;

	setUniformMat4(activeShader, "projMat", projMat);
	setUniformMat4(activeShader, "viewMat", viewMat);

	glBindVertexArray(cubeModel.vao);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)cubeModel.size);
	glEnable(GL_DEPTH_TEST);
}


GLH::OGL_Model GLH::ballModel;


void GLH::loadBallModel()
{
	// change later to non full model
	// only needs verts
	//ballModel = loadModel("res/sphere.obj", 1.f / 0.695000827f);

	memset(&ballModel, 0, sizeof(OGL_Model));
	ballModel.size = sizeof(ballModelVertices) / sizeof(float) / 3;

	glGenVertexArrays(1, &ballModel.vao);
	glBindVertexArray(ballModel.vao);

	glGenBuffers(1, &ballModel.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, ballModel.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ballModelVertices), ballModelVertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3f), (void*)0);
	glEnableVertexAttribArray(0);
}

void GLH::drawCloudBall(Vec3f rot, float height, float radius, float fov, Vec3f move)
{
	Matrix4 projMat = { 0 };
	Matrix4 viewMat = { 0 };

	float aspect = (float)screenW / (float)screenH;

	float far = 1000.f;
	float near = 0.01f;

	projMat(0, 0) = 1.f / (aspect * tanf(toRad(fov) / 2.f));
	projMat(1, 1) = 1.f / tanf(toRad(fov) / 2.f);
	projMat(2, 2) = (far + near) / (near - far);
	projMat(3, 2) = -1.f;
	projMat(2, 3) = -(2.f * far * near) / (far - near);

	Vec3f dir = Vec3f(
		-sinf(toRad(rot.y)) * cosf(toRad(rot.x)),
		sinf(toRad(rot.x)),
		-cosf(toRad(rot.y)) * cosf(toRad(rot.x))
	).normalize();
	Vec3f right = dir.cross(Vec3f(0.f, 1.f, 0.f)).normalize();
	Vec3f up = right.cross(dir);

	Vec3f pos(0.f, height, 0.f);
	viewMat(0, 0) = right.x;
	viewMat(0, 1) = right.y;
	viewMat(0, 2) = right.z;
	viewMat(1, 0) = up.x;
	viewMat(1, 1) = up.y;
	viewMat(1, 2) = up.z;
	viewMat(2, 0) = -dir.x;
	viewMat(2, 1) = -dir.y;
	viewMat(2, 2) = -dir.z;
	viewMat(0, 3) = -right.dot(pos);
	viewMat(1, 3) = -up.dot(pos);
	viewMat(2, 3) = dir.dot(pos);
	viewMat(3, 3) = 1.f;

	Matrix4 mat;
	float rx = toRad(wrapDeg(move.x));
	float ry = toRad(wrapDeg(move.y));
	float rz = toRad(wrapDeg(move.z));

	float ax = radius;
	float ay = radius;
	float az = radius;
	float ox = 0.f;
	float oy = 0.f;
	float oz = 0.f;
	float cx = cosf(rx);
	float cy = cosf(ry);
	float cz = cosf(rz);
	float sx = sinf(rx);
	float sy = sinf(ry);
	float sz = sinf(rz);
	mat(0, 0) = ax * cy * cz;
	mat(0, 1) = ax * cy * sz;
	mat(0, 2) = -ax * sy;
	mat(0, 3) = ox;
	mat(1, 0) = az * sx * sy * cz - ay * cx * sz;
	mat(1, 1) = az * sx * sy * sz + ay * cx * cz;
	mat(1, 2) = az * sx * cy;
	mat(1, 3) = oy;
	mat(2, 0) = az * cx * sy * cz + az * sx * sz;
	mat(2, 1) = az * cx * sy * sz - az * sx * cz;
	mat(2, 2) = az * cx * cy;
	mat(2, 3) = oz;
	mat(3, 3) = 1.f;

	setUniformMat4(activeShader, "projMat", projMat);
	setUniformMat4(activeShader, "viewMat", viewMat);
	setUniformMat4(activeShader, "modelMat", mat);

	glFrontFace(GL_CW);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(ballModel.vao);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)ballModel.size);
	glFrontFace(GL_CCW);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}


GLH::OGL_RenderBuffer GLH::createRenderBuffer(size_t width, size_t height, bool hasDepth)
{
	OGL_RenderBuffer buf = { 0 };

	resizeRenderBuffer(buf, width, height, hasDepth);
	
	return buf;
}

void GLH::resizeRenderBuffer(OGL_RenderBuffer& buf, size_t width, size_t height, bool hasDepth)
{
	deleteRenderBuffer(buf);

	buf.width = width;
	buf.height = height;

	glGenFramebuffers(1, &buf.framebuf);
	glBindFramebuffer(GL_FRAMEBUFFER, buf.framebuf);

	glGenTextures(1, &buf.texture);
	glBindTexture(GL_TEXTURE_2D, buf.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buf.texture, 0);

	glGenRenderbuffers(1, &buf.renderBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, buf.renderBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, buf.renderBuf);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		puts("render buf not complete");
		deleteRenderBuffer(buf);
	}

	glBindRenderbuffer(GL_FRAMEBUFFER, 0);
}

void GLH::drawRenderBuffer(const OGL_RenderBuffer& renderBuf)
{
	useShader(renderBufferShader);
	useTexture(renderBuf.texture);
	glBindVertexArray(screenModel.vao);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)screenModel.size);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void GLH::useRenderBuffer(const OGL_RenderBuffer& renderBuf)
{
	glBindFramebuffer(GL_FRAMEBUFFER, renderBuf.framebuf);
	glViewport(0, 0, renderBuf.width, renderBuf.height);
}

GLH::OGL_RenderBuffer GLH::screenBuf = { 0 };

void GLH::deleteRenderBuffer(OGL_RenderBuffer& renderBuf)
{
	glDeleteFramebuffers(1, &renderBuf.framebuf);
	glDeleteRenderbuffers(1, &renderBuf.renderBuf);
	memset(&renderBuf, 0, sizeof(OGL_RenderBuffer));
}


GLuint GLH::renderBufferShader = 0;


void GLH::loadRenderBufferShader()
{
	const char* vert = 
		"#version 330 core\n"
		"layout(location = 0) in vec2 aPos;\n"
		"out vec2 TexCoord;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = vec4(aPos, 0.f, 1.f);\n"
		"	TexCoord = aPos * 0.5f + 0.5f;\n"
		"}\n";

	const char* frag = 
		"#version 330 core\n"
		"in vec2 TexCoord;\n"
		"out vec4 FragColor;\n"
		"uniform sampler2D renderedTexture;\n"
		"void main()\n"
		"{\n"
		"	FragColor = texture(renderedTexture, TexCoord);\n"
		"}\n";

	renderBufferShader = loadShaderSrc(vert, frag);
}


GLH::OGL_Model GLH::screenModel = { 0 };

void GLH::loadScreenModel()
{
	memset(&screenModel, 0, sizeof(OGL_Model));
	screenModel.size = sizeof(screenVerticies) / sizeof(float);

	glGenVertexArrays(1, &screenModel.vao);
	glBindVertexArray(screenModel.vao);

	glGenBuffers(1, &screenModel.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, screenModel.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVerticies), screenVerticies, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2f), (void*)0);
	glEnableVertexAttribArray(0);
}
