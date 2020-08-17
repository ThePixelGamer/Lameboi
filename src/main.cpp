#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS //imgui_mem_editor

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <vector>

#include "Gameboy/Gameboy.h"
#include "Debug/VRAMViewer.h"
#include "Debug/Debugger.h"

#include "Util/ImGuiHeaders.h"
#include "Util/Texture.h"

#include "PFD.h"
#include "imgui_mem_editor.h"

void displayImage(void* texture, u32 width, u32 height, u32 zoom_mult = 1, std::string name = "Image", bool grid = true, std::function<void (const ImVec2&, const ImVec2&, u32)> extraLines = nullptr) {
	width *= zoom_mult; 
	height *= zoom_mult;

	ImGuiIO& io = ImGui::GetIO();

	ImGui::Text((name + ": %dx%d").c_str(), width, height);
	ImGui::Image(texture, ImVec2(float(width), float(height)), ImVec2(0,0), ImVec2(1,1), ImVec4(1.0f,1.0f,1.0f,1.0f), ImVec4(1.0f,1.0f,1.0f,0.5f));
	
	ImVec2 topleft = ImGui::GetItemRectMin();
	ImVec2 bottomright = ImGui::GetItemRectMax();
	
	if (grid) {
		float line_dist = 8.0f * zoom_mult;

		for (float x = topleft.x + line_dist; x < bottomright.x - line_dist; x += line_dist) {
			ImGui::GetWindowDrawList()->AddLine(ImVec2(x, topleft.y), ImVec2(x, bottomright.y), IM_COL32(169, 169, 169, 255));
		}

		for (float y = topleft.y + line_dist; y < bottomright.y - line_dist; y += line_dist) {
			ImGui::GetWindowDrawList()->AddLine(ImVec2(topleft.x, y), ImVec2(bottomright.x, y), IM_COL32(169, 169, 169, 255));
		}

		if (extraLines) {
			extraLines(topleft, bottomright, zoom_mult);
		}

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();

			float zoom = 4.0f;
			float region_sz = 8.0f * zoom_mult; //region = 8x8 area

			u32 region_x = std::clamp(u32((io.MousePos.x - topleft.x) / region_sz), 0u, width);
			u32 region_y = std::clamp(u32((io.MousePos.y - topleft.y) / region_sz), 0u, height);
			ImVec2 uv0 = ImVec2((region_x * region_sz) / width, (region_y * region_sz) / height);
			ImVec2 uv1 = ImVec2(((region_x + 1) * region_sz) / width, ((region_y + 1) * region_sz) / height);

			ImGui::Text("Coordinate: (%d, %d)", region_x + 1, region_y + 1);
			ImGui::Image(texture, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1);

			ImGui::EndTooltip();
		}
	}
}

int setupGLFW(GLFWwindow** window, const char** glsl_version) {
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Decide GL+GLSL versions
#if __APPLE__
	// GL 3.2 + GLSL 150
	*glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	*glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
	//glfwWindowHint(GLFW_DECORATED, 0); //I want to add my own custom title bar, and how it's styled

	// Create window with graphics context
	*window = glfwCreateWindow(1280, 720, "Lameboi", NULL, NULL);
	if (*window == NULL)
		return 1;
	glfwMakeContextCurrent(*window);
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
	if (err) {
		fprintf(stderr, "Failed to initialize OpenGL loader!\n");
		return 1;
	}

	return 0;
}

int main(int, char**) {
	GLFWwindow* window;
	const char* glsl_version;

	{
		int error = setupGLFW(&window, &glsl_version);
		if (error != 0) {
			return error;
		}
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Our state
	bool show_demo_window = true;
	bool show_emu_window = true;
	bool show_debug_window = false;
	bool show_cpu_window = false;
	bool show_ppu_window = false;
	bool show_bgmap_window = false;
	bool show_tiledata_window = false;
	bool show_oamdata_window = false;
	bool show_display_window = false;
	bool show_mem_window = false;

	bool bgmap = false;
	bool tileset = true;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	size_t steps = 1;

	static MemoryEditor mem_edit;

	auto gb = std::make_unique<Gameboy>(); //avoid stack overflow
	auto vramViewer = std::make_unique<VRAMViewer>();

	Texture bgmapTex = genTexture(256, 256, vramViewer->bgmap.data(), "Background Map 0");
	Texture tiledataTex = genTexture(128, 64 * 3, vramViewer->tileData.data(), "TileData");
	Texture oamTex = genTexture(64, 40, vramViewer->oamData.data(), "OAM");
	Texture displayTex = genTexture(160, 144, gb->ppu.display.data(), "Display");

	const std::vector<std::string> gameboyFileTypes {
		"Gameboy ROMs (.gb)", "*.gb", 
		"All Files", "*"
	};

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


		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Lameboi");

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			
			static std::shared_ptr<pfd::open_file> open_file;

			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, (bool)open_file);

			if (ImGui::Button("Open File")) {
				gb->debug.continuing(false);
				gb->running = false;

				open_file = std::make_shared<pfd::open_file>("Choose file", "C:\\", gameboyFileTypes);
			}

			// this only works because the file dialog takes a while to choose one :)
			if (open_file && open_file->ready()) {
				//wait for the any emu threads to finish
				{
					std::unique_lock lock(gb->emustart_m);
					gb->emustart.wait(lock, [&gb] { return gb->finished; });
				}

				auto result = open_file->result();
				if (!result.empty()) {
					std::string& file = result[0];

					std::cout << "Opened file " << file << "\n";
					std::ifstream rom(file, std::ifstream::binary);

					gb->LoadRom(rom);
					rom.close();

					if (!show_debug_window) {
						gb->debug.continuing(true);
					}

					std::thread emuthread(&Gameboy::Start, gb.get());
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

			if(ImGui::Button("Show PPU"))
				show_ppu_window = !show_ppu_window;

			if(ImGui::Button("Show Debugger")) {
				show_debug_window = !show_debug_window;
				gb->debug.continuing((show_debug_window) ? false : gb->debug.isContinuing());
			}

			ImGui::End();
		}

		if(show_debug_window) {
			static u16 u16_one = 1;
			static u64 u64_one = 1;

			//CPU
			ImGui::Begin("CPU", &show_cpu_window);
			ImGui::InputScalar("Program Counter", ImGuiDataType_U16, &gb->cpu.PC, &u16_one, NULL, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::InputScalar("Stack Pointer", ImGuiDataType_U16, &gb->cpu.SP, &u16_one, NULL, "%04X", ImGuiInputTextFlags_CharsHexadecimal);
			ImGui::Text("0x%02X", gb->cpu.opcode); ImGui::SameLine(); ImGui::Text("Opcode");
			ImGui::Text("Z:%d N:%d HC:%d C:%d", gb->cpu.F.Z, gb->cpu.F.N, gb->cpu.F.HC, gb->cpu.F.C); ImGui::SameLine(); ImGui::Text("Flags");
			ImGui::Text("0x%02X", gb->cpu.A); ImGui::SameLine(); ImGui::Text("A");
			ImGui::Text("0x%04X", gb->cpu.BC); ImGui::SameLine(); ImGui::Text("BC");
			ImGui::Text("0x%04X", gb->cpu.DE); ImGui::SameLine(); ImGui::Text("DE");
			ImGui::Text("0x%04X", gb->cpu.HL); ImGui::SameLine(); ImGui::Text("HL");
			ImGui::End();

			//ASM Viewer/Debugger
			ImGui::Begin("Debugger", &show_debug_window);

			if(ImGui::Button("Continue")) {
				gb->debug.continuing(true);
			}

			ImGui::SameLine(); 
			if(ImGui::Button("Pause")) {
				gb->debug.continuing(false);
			}

			ImGui::SameLine(); 
			if(ImGui::Button("Stop")) {
				gb->debug.continuing(false);
				gb->running = false;
			}

			if(ImGui::Button("Step 1")) {
				gb->debug.step(1);
			}

			if(ImGui::Button("Step")) {
				gb->debug.step(steps);
			}

			ImGui::SameLine(); ImGui::InputScalar("", ImGuiDataType_U64, &steps, &u64_one);
			ImGui::End();

			ImGui::Begin("Breakpoints");
			
			u16 new_breakpoint = 0;
		
			if(ImGui::InputScalar("PC Break", ImGuiDataType_U16, &new_breakpoint, NULL, NULL, "%X", ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
				gb->debug.addBreakpoint(new_breakpoint);
				new_breakpoint = 0;
			}
			
			ImGui::ListBoxHeader("");

			for (auto i = gb->debug.getBreakpoints().begin(); i != gb->debug.getBreakpoints().end();) {
				std::stringstream ss;
				ss << std::hex << *i;
				std::string i_hex = ss.str();

				if (ImGui::Selectable(i_hex.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick) 
					&& ImGui::IsMouseDoubleClicked(0)) {
					gb->debug.removeBreakpoint(i);
				}
				else {
					++i;
				}
			}

			ImGui::ListBoxFooter();

			ImGui::End();

			//Memory
			//mem_edit.DrawWindow("Rom Bank 0", &mem.mbc->romBank0, 0x4000);
			//mem_edit.DrawWindow("Rom Bank N", mem.mbc->current, 0x4000, 0x4000);
			//mem_edit.DrawWindow("Memory", &gb->mem, sizeof(gb->mem) - sizeof(&gb->mem.gb), 0x8000);
		
		}
	
		if(show_ppu_window) {
			//todo allow users to close the windows individually 

			ImGui::Begin("Background Maps", &show_bgmap_window);
			
			if (ImGui::Button("Background Map"))
				bgmap = !bgmap;
			ImGui::SameLine(); ImGui::Text("0x%04X", (bgmap) ? 0x9C00 : 0x9800);

			if (ImGui::Button("Tile Data"))
				tileset = !tileset;
			ImGui::SameLine(); ImGui::Text("0x%04X", (tileset) ? 0x8000 : 0x8800);

			gb->ppu.dumpBGMap(vramViewer->bgmap, bgmap, tileset);
			updateTextures(bgmapTex);

			displayImage((void*)(intptr_t)bgmapTex.texture, bgmapTex.width, bgmapTex.height, 2, bgmapTex.name, true,
				[&gb](const ImVec2& topleft, const ImVec2& bottomright, u32 mult) {
					//const auto boxColor = IM_COL32(105, 105, 105, 255);
					const auto boxColor = IM_COL32(255, 0, 0, 255);
					const auto boxThickness = 1.5f;

					float scx = topleft.x + (gb->mem.SCX * mult);
					float scy = topleft.y + (gb->mem.SCY * mult);

					float boxXSize = 
						(gb->mem.SCX < 96)
							? topleft.x + ((gb->mem.SCX + 160) * mult)
						: topleft.x + ((gb->mem.SCX - 96) * mult);

					float boxYSize = 
						(gb->mem.SCY < 112)
							? topleft.y + ((gb->mem.SCY + 144) * mult)
						: boxYSize = topleft.y + ((gb->mem.SCY - 112) * mult);

					ImVec2 boxTL = ImVec2(scx, scy),
						boxTR = ImVec2(boxXSize, scy),
						boxBL = ImVec2(scx, boxYSize),
						boxBR = ImVec2(boxXSize, boxYSize);

					ImVec2 boxWrapTR = ImVec2(boxXSize, scy),
						boxWrapBL = ImVec2(scx, boxYSize),
						boxWrapXBR = ImVec2(boxXSize, boxYSize),
						boxWrapYBR = ImVec2(boxXSize, boxYSize);

					if(gb->mem.SCX >= 96) {
						//wrap the box on the x axis
						boxTR = ImVec2(boxXSize, scy);

						ImGui::GetWindowDrawList()->AddLine(ImVec2(topleft.x, scy), boxTR, boxColor, boxThickness); //top
						ImGui::GetWindowDrawList()->AddLine(ImVec2(topleft.x, boxYSize), boxBR, boxColor, boxThickness); //bottom

						boxWrapTR = ImVec2(bottomright.x, scy);
						boxWrapXBR = ImVec2(bottomright.x, boxYSize);
					}

					if (gb->mem.SCY >= 112) {
						//wrap the box on the y axis
						boxBL = ImVec2(scx, boxYSize);

						ImGui::GetWindowDrawList()->AddLine(ImVec2(scx, topleft.y), boxBL, boxColor, boxThickness); //left
						ImGui::GetWindowDrawList()->AddLine(ImVec2(boxXSize, topleft.y), boxBR, boxColor, boxThickness); //right

						boxWrapBL = ImVec2(scx, bottomright.y);
						boxWrapYBR = ImVec2(boxXSize, bottomright.y);
					}

					ImGui::GetWindowDrawList()->AddLine(boxTL, boxWrapTR, boxColor, boxThickness); //top
					ImGui::GetWindowDrawList()->AddLine(boxTL, boxWrapBL, boxColor, boxThickness); //left
					ImGui::GetWindowDrawList()->AddLine(boxTR, boxWrapYBR, boxColor, boxThickness); //right
					ImGui::GetWindowDrawList()->AddLine(boxBL, boxWrapXBR, boxColor, boxThickness); //bottom
				}
			);

			ImGui::End();
			

			ImGui::Begin("Tile Data", &show_tiledata_window);

			gb->ppu.dumpTileMap(vramViewer->tileData);
			updateTextures(tiledataTex);
			displayImage((void*)(intptr_t)tiledataTex.texture, tiledataTex.width, tiledataTex.height, 3, tiledataTex.name, true,
				[](const ImVec2& topleft, const ImVec2& bottomright, u32 mult) {
					const auto boxColor = IM_COL32(105, 105, 105, 255);
					//const auto boxColor = IM_COL32(255, 0, 0, 255);
					const auto boxThickness = 1.5f;

					for (int i = 0; i < 3; ++i) {
						float y = topleft.y + (i * 64 * mult);

						ImVec2 lineL = ImVec2(topleft.x, y),
							lineR = ImVec2(bottomright.x, y);

						ImGui::GetWindowDrawList()->AddLine(lineL, lineR, boxColor, boxThickness);
					}
				}
			);

			ImGui::End();


			ImGui::Begin("OAM", &show_oamdata_window);

			gb->ppu.dumpSprites(vramViewer->oamData);
			updateTextures(oamTex);
			displayImage((void*)(intptr_t)oamTex.texture, oamTex.width, oamTex.height, 3, oamTex.name);

			ImGui::End();


			ImGui::Begin("Display", &show_display_window);

			{
				updateTextures(displayTex);
				displayImage((void*)(intptr_t)displayTex.texture, displayTex.width, displayTex.height, 3, displayTex.name, false);

				std::unique_lock lock(gb->ppu.vblank_m);
				gb->ppu.isVblank = true;
				gb->ppu.vblank.notify_one();
			}

			ImGui::End();

			//ImGui::Begin("OAM");
			//
			//updateTextures(oam);
			//displayImage((void*)(intptr_t)oam.texture, oam.width, oam.height, 2, oam.name);
			//
			//ImGui::End();
		}
		
		//Input, not hiding this behind an if statement because it would block inputs
		{
			ImGui::Begin("Inputs");

			//maybe add support for different types of "pressing" to activate?
			auto handleInput = [&gb, &window](int keycode, Button button) -> bool {
				const int key = glfwGetKey(window, keycode);
				if (key == GLFW_PRESS) {
					gb->pad.pressButton(button);
					return true;
				}
				else {
					gb->pad.releaseButton(button);
					return false;
				}
			};

			if (handleInput(GLFW_KEY_W, Button::Up)) {
				ImGui::Text("Up");
			}

			if (handleInput(GLFW_KEY_S, Button::Down)) {
				ImGui::Text("Down");
			}

			if (handleInput(GLFW_KEY_A, Button::Left)) {
				ImGui::Text("Left");
			}

			if (handleInput(GLFW_KEY_D, Button::Right)) {
				ImGui::Text("Right");
			}

			if (handleInput(GLFW_KEY_K, Button::B)) {
				ImGui::Text("B");
			}

			if (handleInput(GLFW_KEY_L, Button::A)) {
				ImGui::Text("A");
			}

			if (handleInput(GLFW_KEY_ENTER, Button::Start)) {
				ImGui::Text("Start");
			}

			if (handleInput(GLFW_KEY_E, Button::Select)) {
				ImGui::Text("Select");
			}

			ImGui::End();
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

	gb->running = false;

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
