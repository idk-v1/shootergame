#include <SDL3/SDL.h>
#include <glad/glad.h>

#include "Engine/OpenGL_helper.h"
#include "Engine/vectorMath.h"
#include "Engine/Entity.h"
#include "Engine/Player.h"


int main()
{
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("Engine test", 600, 600, 
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GLContext glCtx = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(0); // 0 - free refresh / 1 - vsync
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

	GLH::setViewSize(600, 600);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);

	GLuint shader = GLH::loadShader("res/shader.vert", "res/shader.frag");

	GLH::OGL_Model carnoModel = GLH::loadModel("res/carno.obj", 0.02f);
	GLuint carnoTex = GLH::loadTexture("res/carno.png");
	GLH::OGL_Model pterModel = GLH::loadModel("res/pter.obj", 0.1f);
	GLuint pterTex = GLH::loadTexture("res/pter.png");
	GLH::OGL_Model ankyModel = GLH::loadModel("res/anky.obj", 0.1f);
	GLuint ankyTex = GLH::loadTexture("res/anky.png");
	GLH::OGL_Model rhampModel = GLH::loadModel("res/rhamp.obj", 1.f);
	GLuint rhampTex = GLH::loadTexture("res/rhamp.png");

	GLH::Entity player(GLH::Vec3f(0.f, 0.f, 0.f), GLH::Vec3f(0.f, 0.f, 0.f), 0.01f);
	float lookSpeed = 2.f;
	float fov = 90.f;

	Uint64 lastTime = SDL_GetTicks();
	Uint64 deltaTime = 0;
	Uint64 lastFPSTime = lastTime;
	Uint64 fpsCount = 0;

	GLH::useShader(shader);

	//GLH::addAmbientLight(GLH::Vec3f(0.1f, 0.1f, 0.1f));
	//GLH::addPointLight(GLH::Vec3f(1.0f, 0.7f, 0.0f), GLH::Vec3f(0.f, 10.f, 0.f), 100.f);
	GLH::addPointLight(GLH::Vec3f(0.0f, 0.7f, 1.0f), GLH::Vec3f(10.f, 10.f, 0.f), 1000.f);
	GLH::addPointLight(GLH::Vec3f(1.0f, 0.0f, 0.0f), GLH::Vec3f(0.f, 10.f, 10.f), 1000.f);

	bool running = true;
	while (running)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_EVENT_QUIT:
				running = false;
				break;

			case SDL_EVENT_WINDOW_RESIZED:
				GLH::setViewSize(event.window.data1, event.window.data2);
				break;
			}
		}

		Uint64 nowTime = SDL_GetTicks();
		deltaTime += nowTime - lastTime;
		while (deltaTime >= 1000 / 30)
		{
			deltaTime -= 1000 / 30;

			playerController(player, lookSpeed, fov);
			player.updatePhysics(1.1f);

			GLH::setUniformFloat(shader, "time", (float)(lastTime / 100.f));
			GLH::updateCamera(shader, player.pos, player.rot, fov);
		}

		GLH::clear(0, 0, 0);

		GLH::useShader(shader);

		int gridSize = 10;
		//GLH::useTexture(carnoTex);
		//for (int x = -gridSize; x < gridSize; ++x)
		//	for (int z = -gridSize + 0; z < gridSize; z += 4)
		//		GLH::drawModel(carnoModel,
		//			GLH::Vec3f(x * 10.f, 0.f, z * 10.f),
		//			GLH::Vec3f(((x + z) * 10.f + lastTime / 10.f), 0.f, 0.f),
		//			GLH::Vec3f(1.f, 1.f, 1.f));

		//GLH::useTexture(pterTex);
		//for (int x = -gridSize; x < gridSize; ++x)
		//	for (int z = -gridSize + 1; z < gridSize; z += 4)
		//		GLH::drawModel(pterModel,
		//			GLH::Vec3f(x * 10.f, 0.f, z * 10.f),
		//			GLH::Vec3f(0.f, 0.f, ((x + z) * 10.f + lastTime / 10.f)),
		//			GLH::Vec3f(1.f, 1.f, 1.f));

		//GLH::useTexture(ankyTex);
		//for (int x = -gridSize; x < gridSize; ++x)
		//	for (int z = -gridSize + 2; z < gridSize; z += 4)
		//		GLH::drawModel(ankyModel,
		//			GLH::Vec3f(x * 10.f, 0.f, z * 10.f),
		//			GLH::Vec3f(0.f, ((x + z) * 10.f + lastTime / 10.f), 0.f),
		//			GLH::Vec3f(1.f, 1.f, 1.f));

		GLH::useTexture(rhampTex);
		for (int x = -gridSize; x < gridSize; ++x)
			for (int z = -gridSize + 0; z < gridSize; z += 1)
				GLH::drawModel(rhampModel,
					GLH::Vec3f(x * 10.f, 0.f, z * 10.f),
					GLH::Vec3f(((x + z) * 10.f + lastTime / 10.f), 0.f, ((x + z) * 10.f + lastTime / 10.f)),
					GLH::Vec3f(1.f, 1.f, 1.f));

		SDL_GL_SwapWindow(window);
		++fpsCount;

		if (nowTime - lastFPSTime >= 1000 / 4)
		{
			lastFPSTime = nowTime;
			SDL_SetWindowTitle(window, std::to_string(fpsCount * 4).data());
			fpsCount = 0;
		}
		lastTime = nowTime;
	}

	GLH::unloadShader(shader);

	GLH::unloadModel(carnoModel);
	GLH::unloadTexture(carnoTex);
	GLH::unloadModel(pterModel);
	GLH::unloadTexture(pterTex);
	GLH::unloadModel(ankyModel);
	GLH::unloadTexture(ankyTex);
	GLH::unloadModel(rhampModel);
	GLH::unloadTexture(rhampTex);

	SDL_DestroyWindow(window);
	SDL_GL_DestroyContext(glCtx);

	SDL_Quit();

	return 0;
}