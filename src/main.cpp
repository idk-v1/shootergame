#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <filesystem>
#include <climits>

#include "Engine/OpenGL_helper.h"
#include "Engine/vectorMath.h"
#include "Engine/Entity.h"
#include "Engine/Player.h"

std::string getDir();

int main()
{
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


	float cloudRadius = 225.f;
	float cloudLayerDist = 15.f;
	float cloudSpeed = 1.f;
	float groundRadius = 200.f;
	int fancyClouds = 4;
	GLH::loadBallModel();
	GLuint cloudShader = GLH::loadShader(path + "/src/Engine/shaders/cloudbox.vert", path + "/src/Engine/shaders/cloudbox.frag");
	GLuint skyShader = GLH::loadShader(path + "/src/Engine/shaders/sky.vert", path + "/src/Engine/shaders/sky.frag");


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

				GLH::Vec3f sunPos(0.f, sinf(ticks / 100.f), cosf(ticks / 100.f));
				GLH::Light light;
				light.rgb = GLH::Vec3f(0.8f, 0.7f, 0.7f);
				light.norm = (sunPos).normalize();
				light.spread = 1.f / 0.1f;
				light.type = 2.f;
				GLH::setLight(light, 0);

				int valueDiff = 0;
				if (keys[SDL_SCANCODE_UP])
					valueDiff += 1;
				if (keys[SDL_SCANCODE_DOWN])
					valueDiff -= 1;
				if (keys[SDL_SCANCODE_0]) // cloud layer count
				{
					fancyClouds = std::max(fancyClouds + valueDiff, 0);
					if (fancyClouds == 0)
						puts("Cloud layers: Disabled");
					else
						printf("Cloud layers: %d layers\n", fancyClouds);
				}
				if (keys[SDL_SCANCODE_1]) // cloud layer dist
				{
					cloudLayerDist = cloudLayerDist + valueDiff;
					printf("Cloud layer distance: %f\n", cloudLayerDist);
				}
				if (keys[SDL_SCANCODE_2]) // ground height
				{
					groundRadius = groundRadius + valueDiff;
					printf("Ground height: %f\n", groundRadius);
				}
				if (keys[SDL_SCANCODE_3]) // cloud speed
				{
					cloudSpeed = cloudSpeed + valueDiff * 0.1f;
					printf("Cloud speed: %f\n", cloudSpeed);
				}
			}
		}

		size_t triCount = 0;
		Uint64 renderTimeStart = SDL_GetTicksNS();
		GLH::useRenderBuffer(GLH::screenBuf);
		GLH::clearDepth();

		//GLH::useShader(skyboxShader);
		//triCount += GLH::drawSkybox(player.rot, fov, skybox);
		

		GLH::useRenderBuffer(quarterResBuf);

		GLH::useShader(skyShader);
		triCount += GLH::drawSkyBall(player.rot, fov, ticks / 100.f);

		GLH::useShader(cloudShader);
		for (int i = fancyClouds - 1; i >= 0; --i)
			triCount += GLH::drawCloudBall(player.rot, groundRadius,
				cloudRadius + cloudLayerDist * i, fov, 
				ticks * cloudSpeed / (30.f + 20.f * i) * 0.5f,
				ticks * cloudSpeed / (30.f + 20.f * i) * 1.0f,
				ticks / 100.f);

		GLH::useRenderBuffer(GLH::screenBuf);
		triCount += GLH::drawRenderBuffer(quarterResBuf, false);

		GLH::useShader(skyboxShader);
		triCount += GLH::drawSkybox(player.rot, fov, skyboxTerrain);

		GLH::useShader(shader);
		GLH::updateCamera(player.pos, player.rot, fov);

		int gridSize = 10;

		for (int x = -gridSize; x < gridSize; x += 6)
			for (int y = -gridSize; y < gridSize; y += 4)
				for (int z = -gridSize; z < gridSize; z += 4)
				{
					triCount += GLH::drawModelLOD(pterModel, pterTex,
						GLH::Vec3f(x * 10.f, y * 10.f, z * 10.f),
						GLH::Vec3f(((x + z) * 10.f + ticks), 0.f, ((x + z) * 10.f + ticks))) / 3;
				}


		SDL_GL_SwapWindow(window);
		Uint64 renderTimeEnd = SDL_GetTicksNS();
		++fpsCount;

		if (nowTime - lastFPSTime >= 1000 / 4)
		{
			lastFPSTime = nowTime;
			
			std::string triCountFmt = std::to_string(triCount);
			for (int i = (int)triCountFmt.size() - 3; i > 0; i -= 3)
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

	GLH::unloadModelLOD(pterModel);
	GLH::unloadTexture(pterTex);

	GLH::unloadShader(cloudShader);
	GLH::unloadShader(skyShader);
	GLH::unloadModel(GLH::ballModel);
	
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
	std::filesystem::path dir = std::filesystem::current_path();
	while (true)
	{
		for (auto const& entry : std::filesystem::directory_iterator(dir))
		{
			if (entry.is_directory())
				if (entry.path().filename().string() == "res")
					return dir.string();
		}
		if (dir.string() != dir.parent_path().string())
			dir = dir.parent_path();
		else
		{
			puts("Unable to find res folder");
			return "";
		}
	}
}
