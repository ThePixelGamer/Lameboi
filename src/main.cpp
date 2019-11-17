// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan graphics context creation, etc.)

#define _CRT_SECURE_NO_WARNINGS

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "Gameboy/Gameboy.h"

#include <stdio.h>
#include <vector>
#include "PFD.h"
#include <fstream>
#include <math.h>

#include "imgui_mem_editor.h"

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>    // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>    // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>  // Initialize with gladLoadGL()
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct texture {
	GLuint texture;
	GLsizei width, height;
	u32* data;
	const char* name;
};

std::vector<texture> textures;

void genTexture(GLsizei width, GLsizei height, u32* pixels, const char* name) {
	textures.push_back({0, width, height, pixels, name});
	glGenTextures(1, &textures.back().texture);
	glBindTexture(GL_TEXTURE_2D, textures.back().texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void updateTextures(texture& tex) {
	glBindTexture(GL_TEXTURE_2D, tex.texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tex.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void displayImage(void* texture, u32 width, u32 height, u32 zoom_mult = 1, std::string name = "Image") {
	width *= zoom_mult; height *= zoom_mult;
	
	ImGuiIO& io = ImGui::GetIO();
	ImGui::Text((name + ": %dx%d").c_str(), width, height);
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImGui::Image(texture, ImVec2(float(width), float(height)), ImVec2(0,0), ImVec2(1,1), ImVec4(1.0f,1.0f,1.0f,1.0f), ImVec4(1.0f,1.0f,1.0f,0.5f));
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		float region_sz = 8.0f * zoom_mult; //region = 8x8 area
		int region_x = int((io.MousePos.x - pos.x - region_sz * (zoom_mult / 2)) / region_sz); if(region_x < 0) region_x = 0; else if(region_x > 30) region_x = 30;
		int region_y = int((io.MousePos.y - pos.y - region_sz * (zoom_mult / 2)) / region_sz); if(region_y < 0) region_y = 0; else if(region_y > 30) region_y = 30;
		float zoom = 4.0f * zoom_mult;
		ImGui::Text("Coordinate: (%02X, %02X)", ++region_x, ++region_y);
		ImVec2 uv0 = ImVec2((region_x * region_sz) / width, (region_y * region_sz) / height);
		ImVec2 uv1 = ImVec2(((region_x + 1) * region_sz) / width, ((region_y + 1) * region_sz) / height);
		ImGui::Image(texture, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::EndTooltip();
	}
}

int main(int, char**) {
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if(!glfwInit())
		return 1;

	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
	//glfwWindowHint(GLFW_DECORATED, 0); //I want to add my own custom title bar, and how it's styled

	// Create window with graphics context
	GLFWwindow* window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
	if(window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
	bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
	bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
	bool err = gladLoadGL() == 0;
#else
	bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
	if(err) {
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	std::cout << "\0\npee pee" << std::endl;

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state
	bool show_demo_window = true;
	bool show_emu_window = true;
	bool show_debug_window = false;
	bool show_cpu_window = false;
	bool show_ppu_window = false;
	bool show_mem_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	static MemoryEditor mem_edit;

	Gameboy* gb = new Gameboy; //avoid stack overflow

	size_t s_grid = 32 * 32 * TILE_SIZE;
	u32* grid = new u32[s_grid]();
	for(u32 i = 0; i < s_grid; i++) {
		if(!(i % 8) || !(i / 8)) {
			grid[i] = 0x00000088;
		}
	}
	//std::fill(gb->ppu.tileData[1].pixels, std::end(gb->ppu.tileData[1].pixels), 0x555555FF);
	//std::fill(gb->ppu.tileData[2].pixels, std::end(gb->ppu.tileData[2].pixels), 0xAAAAAAFF);
	//std::fill(gb->ppu.tileData[3].pixels, std::end(gb->ppu.tileData[3].pixels), 0xFFFFFFFF);

	genTexture(256, 256, gb->ppu.bgMap1, "Display");
	genTexture(256, 256, gb->ppu.bgMap1, "Background Map 1");
	genTexture(256, 256, gb->ppu.bgMap2, "Background Map 2");
	genTexture(128, 192, gb->ppu.tileData, "Tile Data");

	// Main loop
	while(!glfwWindowShouldClose(window)) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if(show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			
			static std::shared_ptr<pfd::open_file> open_file;

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, (bool)open_file);
			if (ImGui::Button("Open File"))
				open_file = std::make_shared<pfd::open_file>("Choose file", "C:\\", std::vector<std::string>{"Gameboy ROMs (.gb)", "*.gb", "All Files", "*"}, true);
			if (open_file && open_file->ready())
			{
				gb->continue_trigger = false; gb->stop_trigger = false;
				gb->emustart.lock(); gb->emustart.unlock(); //wait for the any emu threads to finish

				auto result = open_file->result();
				if(result.size()) {
					std::cout << "Opened file " << result[0] << "\n";
					std::ifstream rom(result[0], std::ifstream::binary);
					gb->LoadRom(rom);
					rom.close();

					if(!show_debug_window) gb->continue_trigger = true;
					gb->stop_trigger = true;
					std::thread emuthread(&Gameboy::Start, gb);
					emuthread.detach();
				}

				open_file = nullptr;
			}
			ImGui::PopItemFlag();
			
			if(ImGui::Button("Show Gameboy")) {
				show_emu_window = !show_emu_window;
			}

			ImGui::End();
		}

		if(show_emu_window) {
			ImGui::Begin("Gameboy", &show_emu_window);
			if(ImGui::Button("Show CPU"))
				show_cpu_window = !show_cpu_window;
			if(ImGui::Button("Show PPU"))
				show_ppu_window = !show_ppu_window;
			if(ImGui::Button("Show Memory"))
				show_mem_window = !show_mem_window;
			if(ImGui::Button("Show Debugger")) {
				show_debug_window = !show_debug_window;
				gb->continue_trigger = (show_debug_window) ? false : gb->continue_trigger;
			}
			ImGui::End();
		}

		if(show_cpu_window) {
			static u16 u16_one = 1;

			ImGui::Begin("CPU", &show_cpu_window);
			ImGui::InputScalar("Program Counter", ImGuiDataType_U16, &gb->cpu.PC, &u16_one, NULL, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::InputScalar("Stack Pointer", ImGuiDataType_U16, &gb->cpu.SP, &u16_one, NULL, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::Text("0x%02X", gb->cpu.opcode); ImGui::SameLine(); ImGui::Text("Opcode");
			ImGui::Text("Z:%d N:%d HC:%d C:%d", gb->cpu.F & 0x80, gb->cpu.F & 0x40, gb->cpu.F & 0x20, gb->cpu.F & 0x10); ImGui::SameLine(); ImGui::Text("Flags");
			ImGui::Text("0x%02X", gb->cpu.A); ImGui::SameLine(); ImGui::Text("A");
			ImGui::Text("0x%04X", gb->cpu.BC); ImGui::SameLine(); ImGui::Text("BC");
			ImGui::Text("0x%04X", gb->cpu.DE); ImGui::SameLine(); ImGui::Text("DE");
			ImGui::Text("0x%04X", gb->cpu.HL); ImGui::SameLine(); ImGui::Text("HL");
			ImGui::End();
		}

		if(show_debug_window) {
			static u64 u64_one = 1;

			ImGui::Begin("Debugger", &show_debug_window);

			if(ImGui::Button("Continue")) {
				gb->continue_trigger = true;
			}
			ImGui::SameLine(); if(ImGui::Button("Pause")) {
				gb->continue_trigger = false;
			}
			ImGui::SameLine(); if(ImGui::Button("Stop")) {
				gb->continue_trigger = false;
				gb->stop_trigger = false;
			}

			if(ImGui::Button("Step 1")) {
				gb->step = 1;
				gb->step_old = 1;
				gb->continue_trigger = false;
				gb->step_trigger = true;
			}
			if(ImGui::Button("Step")) {
				gb->step_old = gb->step;
				gb->continue_trigger = false;
				gb->step_trigger = true;
			}

			ImGui::SameLine(); ImGui::InputScalar("", ImGuiDataType_U64, &gb->step, &u64_one);

			ImGui::End();
		}

		if(show_ppu_window) {
			ImGui::Begin("PPU", &show_ppu_window);

			for(size_t i = 1; i < textures.size(); i++) {
				updateTextures(textures[i]);
				displayImage((void*)(intptr_t)textures[i].texture, textures[i].width, textures[i].height, 2, textures[i].name);
			}
			ImGui::End();

			ImGui::Begin("Window");
			updateTextures(textures[0]);
			ImGui::Image((void*)(intptr_t)textures[0].texture, ImVec2(160.0f * 2, 144.0f * 2), ImVec2(gb->mem.SCX / 256.0f, gb->mem.SCY / 256.0f), ImVec2((gb->mem.SCX + 160.0f) / 256.0f, (gb->mem.SCY + 144.0f) / 256.0f));
			ImGui::End();
		}

		if(show_mem_window) {
			//mem_edit.DrawWindow("Rom Bank 0", &mem.mbc->romBank0, 0x4000);
			//mem_edit.DrawWindow("Rom Bank N", mem.mbc->current, 0x4000, 0x4000);
			mem_edit.DrawWindow("Memory", &gb->mem, sizeof(gb->mem) - sizeof(&gb->mem.gb), 0x8000);
		}
		
		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
	}

	gb->continue_trigger = false;
	gb->stop_trigger = false;
	//emuthread.join();

	delete gb;
	delete[] grid;

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
