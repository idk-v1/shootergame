#include "OpenGL_helper.h"
#include "ObjReader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <fstream>
#include <sstream>

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
	else // Needed to be caught
		throw std::runtime_error(std::string("Failed to open file ") + name);

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


GLH::OGL_Model GLH::loadModel(const std::string& name, float scale)
{
	std::vector<GLH::Vertex> data = readObjFile(name);
	if (scale != 1.f)
	{
		for (size_t i = 0; i < data.size(); ++i)
		{
			data[i].pos.x *= scale;
			data[i].pos.y *= scale;
			data[i].pos.z *= scale;
		}
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


bool sphereCollision(GLH::Vec3f aPos, GLH::Vec3f bPos, float aRad, float bRad)
{
	GLH::Vec3f diff = aPos - bPos;
	float dist = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	return (dist < (aRad + bRad) * (aRad + bRad));
}

void GLH::drawModel(const OGL_Model& model, 
	const Vec3f& pos, const Vec3f& rot, const Vec3f& scale)
{
	Matrix4 mat;
	float rx = toRad(rot.x);
	float ry = toRad(rot.y);
	float rz = toRad(rot.z);
	//mat(0, 0) = scale.x;
	//mat(1, 1) = scale.y;
	//mat(2, 2) = scale.z;
	//mat(0, 3) = pos.x;
	//mat(1, 3) = pos.y;
	//mat(2, 3) = pos.z;
	//mat(3, 3) = 1.f;
	
	//Matrix4 rotMatX;
	//rotMatX(0, 0) = 1.f;
	//rotMatX(1, 1) = cosf(rx);
	//rotMatX(2, 1) = -sinf(rx);
	//rotMatX(1, 2) = sinf(rx);
	//rotMatX(2, 2) = cosf(rx);
	//rotMatX(3, 3) = 1.f;
	
	//Matrix4 rotMatY;
	//rotMatY(0, 0) = cosf(ry);
	//rotMatY(2, 0) = sinf(ry);
	//rotMatY(1, 1) = 1.f;
	//rotMatY(0, 2) = -sinf(ry);
	//rotMatY(2, 2) = cosf(ry);
	//rotMatY(3, 3) = 1.f;
	
	//Matrix4 rotMatZ;
	//rotMatZ(0, 0) = cosf(rz);
	//rotMatZ(1, 0) = -sinf(rz);
	//rotMatZ(0, 1) = sinf(rz);
	//rotMatZ(1, 1) = cosf(rz);
	//rotMatZ(2, 2) = 1.f;
	//rotMatZ(3, 3) = 1.f;
	
	//mat = mat * rotMatX * rotMatY * rotMatZ;

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

	glDrawArrays(GL_TRIANGLES, 0, model.size);
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
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

void GLH::setViewSize(size_t w, size_t h)
{
	glViewport(0, 0, w, h);
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
	float skyboxVertices[] = 
	{
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
	};

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

		stbi_uc* subData = (stbi_uc*)malloc(cubeSize * cubeSize * 4);
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
					memcpy(subData + cubeSize * 4 * y, data + (ox + oy) * 4, cubeSize * 4);
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
	glDrawArrays(GL_TRIANGLES, 0, cubeModel.size);
	glEnable(GL_DEPTH_TEST);
}
