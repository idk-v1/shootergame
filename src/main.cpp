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

	GLH::setViewSize(800, 600);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);


	GLuint shader = GLH::loadShader("src/Engine/shaders/main.vert", "src/Engine/shaders/main.frag");
	GLuint skyboxShader = GLH::loadShader("src/Engine/shaders/skybox.vert", "src/Engine/shaders/skybox.frag");

	GLuint skybox = GLH::loadSkybox("res/skybox.png");
	GLuint skyboxTerrain = GLH::loadSkybox("res/skyboxTerrain.png");
	GLH::loadCubeModel();

	GLuint cloudShader = GLH::loadShader("src/Engine/shaders/cloudbox.vert", "src/Engine/shaders/cloudbox.frag");
	GLH::loadBallModel();

	GLH::loadNoTexture();


	GLH::OGL_Model pterModel = GLH::loadModel("res/pter.obj", 0.1f);
	GLuint pterTex = GLH::loadTexture("res/pter.png");


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
				GLH::setViewSize(event.window.data1, event.window.data2);
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

		GLH::clearDepth();

		GLH::useShader(skyboxShader);
		GLH::drawSkybox(player.rot, fov, skybox);
		
		GLH::useShader(cloudShader);
		GLH::drawCloudBall(player.rot, 100.f, 105.f, fov, GLH::Vec3f(0.f, 0.f, ticks / 8.f));

		GLH::useShader(skyboxShader);
		GLH::drawSkybox(player.rot, fov, skyboxTerrain);

		GLH::useShader(shader);
		GLH::updateCamera(player.pos, player.rot, fov);


		int gridSize = 10;
		size_t triCount = 0;

		for (int x = -gridSize; x < gridSize; x += 6)
			for (int y = -gridSize; y < gridSize; y += 4)
				for (int z = -gridSize; z < gridSize; z += 4)
				{
					GLH::drawModel(pterModel, pterTex,
						GLH::Vec3f(x * 10.f, y * 10.f, z * 10.f),
						GLH::Vec3f(((x + z) * 10.f + ticks), 0.f, ((x + z) * 10.f + ticks)),
						GLH::Vec3f(1.f, 1.f, 1.f));
						triCount += pterModel.size / 3;

						//GLH::drawModel(GLH::ballModel, GLH::noTexture, GLH::Vec3f(x * 10.f, y * 10.f, z * 10.f),
						//	GLH::Vec3f(((x + z) * 10.f + ticks), 0.f, ((x + z) * 10.f + ticks)),
						//	GLH::Vec3f(pterModel.boundingRad));
				}

		SDL_GL_SwapWindow(window);
		++fpsCount;

		if (nowTime - lastFPSTime >= 1000 / 4)
		{
			lastFPSTime = nowTime;
			SDL_SetWindowTitle(window, ("FPS: " + std::to_string(fpsCount * 4) + " | " + 
				std::to_string(triCount)).data());
			fpsCount = 0;
		}
		lastTime = nowTime;
	}

	GLH::unloadShader(shader);

	GLH::unloadModel(pterModel);
	GLH::unloadTexture(pterTex);

	GLH::unloadShader(cloudShader);
	GLH::unloadModel(GLH::ballModel);

	GLH::unloadShader(skyboxShader);
	GLH::unloadTexture(skybox);
	GLH::unloadTexture(skyboxTerrain);
	GLH::unloadModel(GLH::cubeModel);

	GLH::unloadTexture(GLH::noTexture);

	SDL_DestroyWindow(window);
	SDL_GL_DestroyContext(glCtx);

	SDL_Quit();

	return 0;
}