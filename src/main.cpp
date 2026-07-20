#include <filesystem>

#include <glad/glad.h>

#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include <SDL3/SDL.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include "core/Gameboy.h"
#include "core/Input.h"
#include "frontend/MainWindow.h"

#define LB_GL_MAJOR 4
#define LB_GL_MINOR 3

// todo: change
#define LB_GL_LOG(ErrorTest, ...) \
	if (ErrorTest) LB_ERROR(GL, __VA_ARGS__); else LB_INFO(GL, __VA_ARGS__);

void GLAPIENTRY GL_DebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	LB_GL_LOG(type == GL_DEBUG_TYPE_ERROR, "type: 0x{:x}, severity: 0x{:x}, message = {}", type, severity, message);
}

class App {
public:
	SDL_Window* window = nullptr;
	SDL_GLContext gl_context = nullptr;

	Gameboy gb;
	UI ui;

	App(SDL_Window* window, SDL_GLContext gl_context) : 
		window(window),
		gl_context(gl_context),
		ui(gb) {
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
		ImGui_ImplOpenGL3_Init("#version 430");

		LB_INFO(App, "Working directory is {}", std::filesystem::current_path().string());
		gb.loadBios(config.biosPath);
	}

	~App() {
		inputManager.gamepad.close();

		shutdown();
	}

	// todo: not happy with this, done to make sure OpenGL is ready for viewportwindow
	static App* create() {
		SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_AUDIO)) {
			LB_ERROR(App, "Unable to initialize SDL: %s", SDL_GetError());
			return nullptr;
		}

		// todo: add support for other platforms?

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, LB_GL_MAJOR);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, LB_GL_MINOR);

		// Create window with graphics context
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
		auto window = SDL_CreateWindow("lameboi", 1280, 720, window_flags);
		auto gl_context = SDL_GL_CreateContext(window);
		SDL_GL_MakeCurrent(window, gl_context);
		//SDL_GL_SetSwapInterval(1); // Enable vsync

		int version = gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);
		if (version == 0) {
			LB_ERROR(App, "Failed to initialize OpenGL context");
			return nullptr;
		}

		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(GL_DebugOutput, 0);

		return new App(window, gl_context);
	}

	void shutdown() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL3_Shutdown();

		ImGui::DestroyContext();

		LB_INFO(App, "App terminated");

		SDL_GL_DestroyContext(gl_context);
		SDL_DestroyWindow(window);
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
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL3_NewFrame();
		ImGui::NewFrame();

		if (ui.render()) {
			return SDL_APP_SUCCESS;
		}

		endFrame();
		return SDL_APP_CONTINUE;
	}

	SDL_AppResult process(SDL_Event* event) {
		ImGui_ImplSDL3_ProcessEvent(event);

		inputManager.processEvent(*event);

		if (event->type == SDL_EVENT_QUIT)
			return SDL_APP_SUCCESS;
		if (event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event->window.windowID == SDL_GetWindowID(window))
			return SDL_APP_SUCCESS;

		return SDL_APP_CONTINUE;
	}
};

// SDL Main Callbacks
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv) {
	if (auto app = App::create()) {
		*appstate = app;
		return SDL_APP_CONTINUE;
	}

	return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppIterate(void* appstate) {
	return static_cast<App*>(appstate)->run();
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) {
	return static_cast<App*>(appstate)->process(event);
}

void SDL_AppQuit(void* appstate, SDL_AppResult result) {
	if (auto app = static_cast<App*>(appstate))
		delete app;
}
