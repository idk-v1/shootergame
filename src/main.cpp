#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <filesystem>
#include <climits>
#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

#include "Engine/OpenGL_helper.h"
#include "Engine/vectorMath.h"
#include "Engine/Entity.h"
#include "Engine/Player.h"

std::string getDir();

int main()
{
	// Linux needs an absolute path; cwd is not where the executable is,
	// its the directory where the program was started,
	// i.e. if I am @ /a/b/c/ and i do /foo/bar/shootergame.o
	// then the CWD is /a/b/c/
	std::string path = getDir();


	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow("Engine test", 600, 600, 
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GLContext glCtx = SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(0); // 0 - free refresh / 1 - vsync
	gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

	GLH::setViewSize(800, 600, GLH::screenBuf);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);


	GLuint shader = GLH::loadShader(path + "/src/Engine/shaders/main.vert", path + "/src/Engine/shaders/main.frag");
	GLuint skyboxShader = GLH::loadShader(path + "/src/Engine/shaders/skybox.vert", path + "/src/Engine/shaders/skybox.frag");

	GLuint skybox = GLH::loadSkybox(path + "/res/skybox.png");
	GLuint skyboxTerrain = GLH::loadSkybox(path + "/res/skyboxTerrain.png");
	GLH::loadCubeModel();


	bool useFancyClouds = true;
	GLuint cloudShader = 0;
	if (useFancyClouds)
	{
		GLH::loadBallModel();
		cloudShader = GLH::loadShader(path + "/src/Engine/shaders/cloudbox.vert", path + "/src/Engine/shaders/cloudbox.frag");
	}

	//GLH::OGL_Model pterModel = GLH::loadModel(path + "/res/pter.obj", 0.1f);
	std::vector<GLH::OGL_Model> pterModel = GLH::loadModelLOD(path + "/res/pter", 4, 0.1f);
	GLuint pterTex = GLH::loadTexture(path + "/res/pter.png");
  
	GLH::Entity player(GLH::Vec3f(0.f, 0.f, 0.f), GLH::Vec3f(0.f, 0.f, 0.f), 0.02f);
	float lookSpeed = 0.1f;
	float fov = 90.f;


	Uint64 lastTime = SDL_GetTicks();
	Uint64 deltaTime = 0;
	Uint64 lastFPSTime = lastTime;
	Uint64 fpsCount = 0;
	Uint64 ticks = 0;


	GLH::useShader(shader);

	GLH::addDirectionalLight(GLH::Vec3f(0.8f, 0.7f, 0.7f), GLH::Vec3f(2.f, 1.f, 1.f).normalize(), 0.1f);
	GLH::addAmbientLight(GLH::Vec3f(0.1f, 0.1f, 0.05f));

	GLH::loadRenderBufferShader();
	GLH::loadScreenModel();
	GLH::OGL_RenderBuffer quarterResBuf = GLH::createRenderBuffer(GLH::screenBuf.width / 4, GLH::screenBuf.height / 4, false);


	bool running = true;
	bool paused = true;
	bool escLast = false;
	bool focused = true;

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
				GLH::setViewSize(event.window.data1, event.window.data2, GLH::screenBuf);
				GLH::resizeRenderBuffer(quarterResBuf, event.window.data1 / 4, event.window.data2 / 4);
				break;

			case SDL_EVENT_WINDOW_FOCUS_LOST:
				if (!paused)
				{
					paused = true;

					SDL_SetWindowRelativeMouseMode(window, false);
					int w = 0, h = 0;
					SDL_GetWindowSize(window, &w, &h);
					SDL_WarpMouseInWindow(window, w / 2.f, h / 2.f);
					SDL_GetRelativeMouseState(NULL, NULL);
					focused = false;
				}
				break;
			case SDL_EVENT_WINDOW_FOCUS_GAINED:
				focused = true;
				break;
			}
		}

		const bool* keys = SDL_GetKeyboardState(NULL);
		bool esc = keys[SDL_SCANCODE_ESCAPE];
		if (esc && !escLast && focused)
		{
			paused = !paused;

			SDL_SetWindowRelativeMouseMode(window, !paused);
			int w = 0, h = 0;
			SDL_GetWindowSize(window, &w, &h);
			SDL_WarpMouseInWindow(window, w / 2.f, h / 2.f);
			SDL_GetRelativeMouseState(NULL, NULL);
		}
		escLast = esc;

		Uint64 nowTime = SDL_GetTicks();
		deltaTime += nowTime - lastTime;
		while (deltaTime >= 1000 / 30)
		{
			deltaTime -= 1000 / 30;

			if (!paused)
			{
				++ticks;
				playerController(player, lookSpeed, fov, window);
				player.updatePhysics(1.1f);
			}
		}

		Uint64 renderTimeStart = SDL_GetTicksNS();
		GLH::useRenderBuffer(GLH::screenBuf);
		GLH::clearDepth();
		GLH::useShader(skyboxShader);
		GLH::drawSkybox(player.rot, fov, skybox);
		
		if (useFancyClouds)
		{
			GLH::useRenderBuffer(quarterResBuf);
			GLH::clear(0.f, 0.f, 0.f, 0.f);
			GLH::useShader(cloudShader);
			GLH::drawCloudBall(player.rot, 200.f, 225.f, fov, GLH::Vec3f(0.f, 0.f, ticks / 30.f));

			GLH::useRenderBuffer(GLH::screenBuf);
			GLH::drawRenderBuffer(quarterResBuf);

			GLH::useShader(skyboxShader);
			GLH::drawSkybox(player.rot, fov, skyboxTerrain);
		}

		GLH::useShader(shader);
		GLH::updateCamera(player.pos, player.rot, fov);


		int gridSize = 10;
		size_t triCount = 0;

		for (int x = -gridSize; x < gridSize; x += 6)
			for (int y = -gridSize; y < gridSize; y += 4)
				for (int z = -gridSize; z < gridSize; z += 4)
				{
					triCount += GLH::drawModelLOD(pterModel, pterTex,
						GLH::Vec3f(x * 10.f, y * 10.f, z * 10.f),
						GLH::Vec3f(((x + z) * 10.f + ticks), 0.f, ((x + z) * 10.f + ticks))) / 3;

					//triCount += GLH::drawModel(pterModel, pterTex,
					//	GLH::Vec3f(x * 10.f, y * 10.f, z * 10.f),
					//	GLH::Vec3f(((x + z) * 10.f + ticks), 0.f, ((x + z) * 10.f + ticks))) / 3;
				}

		SDL_GL_SwapWindow(window);
		Uint64 renderTimeEnd = SDL_GetTicksNS();
		++fpsCount;

		if (nowTime - lastFPSTime >= 1000 / 4)
		{
			lastFPSTime = nowTime;
			
			std::string triCountFmt = std::to_string(triCount);
			for (int i = triCountFmt.size() - 3; i > 0; i -= 3)
				triCountFmt.insert(i, 1, ',');

			Uint64 renderMS = SDL_NS_TO_MS((renderTimeEnd - renderTimeStart) * 10);
			std::string renderTime = (std::to_string(renderMS / 10) + '.') + (char)(renderMS % 10 + '0');

			std::string title =
				"FPS: " + std::to_string(fpsCount * 4) + " | " +
				"Render MS: " + renderTime + " | " +
				"Tri Count: " + triCountFmt;
			SDL_SetWindowTitle(window, title.data());
			
			fpsCount = 0;
		}
		lastTime = nowTime;
	}

	GLH::unloadShader(shader);

	//GLH::unloadModel(pterModel);
	GLH::unloadModelLOD(pterModel);
	GLH::unloadTexture(pterTex);

	//GLH::unloadModel(sphere);
	//GLH::unloadTexture(GLH::noTexture);

	if (useFancyClouds)
	{
		GLH::unloadShader(cloudShader);
		GLH::unloadModel(GLH::ballModel);
	}

	GLH::unloadShader(skyboxShader);
	GLH::unloadTexture(skybox);
	GLH::unloadTexture(skyboxTerrain);
	GLH::unloadModel(GLH::cubeModel);

	GLH::deleteRenderBuffer(quarterResBuf);
	GLH::unloadShader(GLH::renderBufferShader);
	GLH::unloadModel(GLH::screenModel);

	SDL_DestroyWindow(window);
	SDL_GL_DestroyContext(glCtx);

	SDL_Quit();

	return 0;
}


std::string getDir()
{
#ifdef _WIN32
	char dir[MAX_PATH];
	GetModuleFileNameA(NULL, dir, MAX_PATH);
	return std::filesystem::path(dir).parent_path().parent_path().parent_path().string();
#else
	char dir[PATH_MAX + 1];
	size_t count = readlink("/proc/self/exe", dir, PATH_MAX);
	dir[count] = '\0';
	return std::filesystem::path(dir).parent_path().parent_path().string();
#endif
}
