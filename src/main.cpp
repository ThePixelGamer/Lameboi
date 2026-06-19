#include <algorithm>
#include <filesystem>

#include <glad/glad.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include "util/ImGuiHeaders.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include "App.h"
#include "util/FileUtil.h"

SDL_Window* window = nullptr;
SDL_GLContext gl_context = nullptr;

class ImGuiApp : public App {
public:
	ImGuiApp() {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
		ImGui_ImplOpenGL3_Init("#version 130");

		LB_INFO(App, "Working directory is {}", std::filesystem::current_path().string());
	}

	~ImGuiApp() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();

		ImGui::DestroyContext();
	}

	void endFrame() {
		ImGuiIO& io = ImGui::GetIO();

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(window, gl_context);
		}

		SDL_GL_SwapWindow(window);
	}

	SDL_AppResult run() {
		if (requestExit) {
			//quit();
			return SDL_APP_SUCCESS;
		}

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		render();
		ImGui::ShowDemoWindow();

		endFrame();
		return SDL_APP_CONTINUE;
	}
};

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
		LB_ERROR(App, "Unable to initialize SDL: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// todo: add support for other platforms?

	// GL 3.2
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
	window = SDL_CreateWindow("lameboi", 1280, 720, window_flags);
	gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	//SDL_GL_SetSwapInterval(1); // Enable vsync

	int version = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
	if (version == 0) {
		LB_ERROR(App, "Failed to initialize OpenGL context");
		return SDL_APP_FAILURE;
	}

	auto app = new ImGuiApp();
	*appstate = app;
	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	return static_cast<ImGuiApp*>(appstate)->run();
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	auto app = static_cast<ImGuiApp*>(appstate);

	ImGui_ImplSDL3_ProcessEvent(event);
	inputManager.processEvent(*event);

	if (event->type == SDL_EVENT_QUIT)
		app->requestExit = true;
	if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event->window.windowID == SDL_GetWindowID(window))
		app->requestExit = true;

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	delete static_cast<ImGuiApp*>(appstate);

	LB_INFO(App, "App terminated");

	SDL_GL_DestroyContext(gl_context);
	SDL_DestroyWindow(window);
}
