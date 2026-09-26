#include "DisplayWindow.h"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "MainWindow.h"
#include "util/FIFO.h"

namespace ui {

bool operator&(DisplayWindow::Mode a, DisplayWindow::Mode b) {
	return (int)a & (int)b;
}

DisplayWindow::DisplayWindow(UI& context) :
	context(context),
	display() {
}

void DisplayWindow::render() {
	auto& gb = context.gb;

	// todo: combine this with the bgmapwindow one (both are the same currently)
	auto square = [](ImGuiSizeCallbackData* data) {
		auto beforeImage = static_cast<ImVec2*>(data->UserData);
		ImGuiContext& g = *GImGui;
		ImVec2& padding = ImGui::GetStyle().WindowPadding;
		ImVec2 size = data->DesiredSize - *beforeImage - padding;
		size /= ImVec2(Display::W, Display::H);

		float m = std::max(size.x, size.y);

		// todo: open an issue for imgui to provide some sort of context for direction
		if (g.ActiveIdSource == ImGuiInputSource_Mouse) {
			bool resized = true;
			
			// will break with ImGuiConfigFlags_NoMouseCursorChange
			switch (g.MouseCursor) {
				case ImGuiMouseCursor_ResizeEW:
					m = size.x;
					break;

				case ImGuiMouseCursor_ResizeNS:
					m = size.y;
					break;

				case ImGuiMouseCursor_ResizeNWSE:
				case ImGuiMouseCursor_ResizeNESW:
					break;

				default:
					resized = false;
			}

			if (resized && g.IO.KeyShift)
				m = std::round(m);
		}
		else {
			ImVec2 nav_resize_delta;

			if (g.NavInputSource == ImGuiInputSource_Keyboard && g.IO.KeyShift)
				nav_resize_delta = ImGui::GetKeyMagnitude2d(ImGuiKey_LeftArrow, ImGuiKey_RightArrow, ImGuiKey_UpArrow, ImGuiKey_DownArrow);
			if (g.NavInputSource == ImGuiInputSource_Gamepad)
				nav_resize_delta = ImGui::GetKeyMagnitude2d(ImGuiKey_GamepadDpadLeft, ImGuiKey_GamepadDpadRight, ImGuiKey_GamepadDpadUp, ImGuiKey_GamepadDpadDown);

			if (nav_resize_delta.x != 0.0f && nav_resize_delta.y != 0.0f) {
				// m is already set
			}
			else if (nav_resize_delta.x != 0.0f) {
				m = size.x;
			}
			else if (nav_resize_delta.y != 0.0f) {
				m = size.y;
			}
		}

		data->DesiredSize.x = (m * Display::W) + beforeImage->x + padding.x;
		data->DesiredSize.y = (m * Display::H) + beforeImage->y + padding.y;
	};

	//ImVec2 windowMinSize = oldCursor + ImVec2(Display::W, Display::H) + ImGui::GetStyle().WindowPadding;
	//ImGui::SetNextWindowSizeConstraints(windowMinSize, ImVec2(FLT_MAX, FLT_MAX), square, &oldCursor);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0, 0 });
	ImGui::Begin("Lameboi", nullptr, ImGuiWindowFlags_NoDecoration);
	ImGui::PopStyleVar();

	ImGui::Spacing();
	ImGui::SetCursorPosX(ImGui::GetStyle().WindowPadding.x);

	if (mode & Mode::Scene && inFocus) {
		Scene::InputAction action = scene.handleInput(avoidReset);
		
		if (action == Scene::OpenContextMenu) {
			ImGui::OpenPopup("display_context_popup");
			captureMouse(false);
		}
		else if (action == Scene::UnlockMouse) {
			captureMouse(false);
		}
	}

	if (ImGui::Button("Stop")) {
		context.gb.stop();
	}

	// Update FPS counter
	// todo: run on a separate thread to not be affected by UI performance?
	using namespace std::chrono_literals;
	auto perf = std::chrono::duration_cast<std::chrono::seconds>(clock::now() - perfTimer);
	if (perf >= 1s) {
		// add seconds to existing time_point to avoid missing the next second (handle lost remainder from duration_cast)
		perfTimer += perf;
		if (gb.ppu.framesPresented) {
			// take avg in case this takes longer than 1 second to run again
			fps = gb.ppu.framesPresented / perf.count();
			gb.ppu.framesPresented = 0;
		}
		else {
			fps = 0;
		}

		instrCount = gb.cpu.instrCount;
		gb.cpu.instrCount = 0;
	}

	// Not a fan of this
	std::string status{};
	if (gb.emuRun) {
		status = (gb.debug.running) ? fmt::format("{} ips {} fps", instrCount, fps) : "Paused";
	}
	
	if (!status.empty()) {
		ImGui::SameLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImMax(0.0f, ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(status.c_str()).x - ImGui::GetStyle().ItemInnerSpacing.x));
		ImGui::Text("%s", status.c_str());
	}

	oldCursor = ImGui::GetCursorPos();

	// Inputs
	focused = ImGui::IsWindowFocused();

	// Display
	updateBuffer();

	ImVec2 sceneSize = ImGui::GetContentRegionAvail();
	ImVec2 displaySize = ImGui::GetContentRegionAvail();
	if (mode == Mode::SideBySide) {
		sceneSize.x /= 2;
		displaySize.x /= 2;
	}

	if (mode & Mode::Scene) {
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
		if (ImGui::ImageButton("##viewport", (ImTextureID)scene.render(sceneSize.x, sceneSize.y), sceneSize, ImVec2(0, 1), ImVec2(1, 0))) {
			captureMouse(true);
		}
		ImGui::PopStyleVar();

		if (ImGui::BeginPopupContextItem("scene_ctx")) {
			ImGui::Checkbox("Wireframe", &scene.wireframe);
			ImGui::SliderFloat("FOV", &scene.fov, 30.0f, 150.0f, "%.0f");
			
			ImGui::Separator();
			contextMenu();
			
			ImGui::EndPopup();
		}
	}

	if (mode == Mode::SideBySide) {
		ImGui::SameLine();
	}

	if (mode & Mode::Display) {
		display.render(std::max(std::min(displaySize.x / Display::W, displaySize.y / Display::H), 1.0f));
	
		if (ImGui::BeginPopupContextItem("display_ctx")) {
			if (ImGui::MenuItem("Use Custom Graphics", nullptr, &useCG)) {
				gb.ppu.redraw = true;
			}

			ImGui::Separator();
			contextMenu();
			
			ImGui::EndPopup();
		}
	}


	ImGui::End();
}

void DisplayWindow::contextMenu() {
	auto& gb = context.gb;

	if (ImGui::MenuItem("Show Scene", nullptr, mode & Mode::Scene)) {
		gb.ppu.redraw = true;
	}
	if (ImGui::MenuItem("Fullscreen", nullptr, &fullscreen)) {
		SDL_SetWindowFullscreenMode(SDL_GL_GetCurrentWindow(), nullptr);
		SDL_SetWindowFullscreen(SDL_GL_GetCurrentWindow(), fullscreen);
	}
}

void DisplayWindow::updateBuffer() {
	auto& ppu = context.gb.ppu;

	Framebuffer* buffer = ppu.getNextBuffer();
	if (buffer) {
		if (mode & Mode::Scene) {
			renderScene(*buffer);
		}

		if (mode & Mode::Display) {
			if (useCG) {
				renderCG(*buffer);
			}
			else {
				for (size_t p = 0; p < (display.W * display.H); ++p) {
					const Color pixel = buffer->pixels[p].getColor(ppu.model);

					size_t idx = p * 4;
					display.data()[idx] = pixel.r;
					display.data()[idx + 1] = pixel.g;
					display.data()[idx + 2] = pixel.b;
					display.data()[idx + 3] = pixel.a;
				}
			}
		}
	}
}

void DisplayWindow::renderCG(Framebuffer& buffer) {
	auto& ppu = context.gb.ppu;
	
	u8 windowLines = 0;
	for (size_t y = 0; y < 144; ++y) {
		auto& meta = buffer.meta;
		auto& line = meta.lines[y];

		struct Pixel {
			Color col;
			bool valid = true;
		};

		FIFO<Pixel, 8> bg;

		size_t mapIdx = (line.bg.altMap) ? 1024 : 0;
		size_t tileIdx = 0;
		u8 x = (line.bg.x / 8) & 0x1F;
		u16 yOffset = ((y + line.bg.y) / 8) * 32;
		
		for (size_t p = 0; p < 160; ++p) {
			if (line.window.enabled && std::max(line.window.x - 7, 0) == p && line.window.y <= y) {
				mapIdx = (line.window.altMap) ? 1024 : 0;
				x = 0;
				yOffset = (windowLines++ / 8) * 32;
			}

			if (bg.size() == 0) {
				auto& tile = meta.maps[mapIdx + yOffset + x];
				x = (x + 1) & 0x1F;

				tileIdx = [&]() {
					u16 idx = 384 * tile.altBank;

					if (tile.idx & 0x80) {
						idx += 128;
					}
					else if (!line.altTileSet) {
						idx += 256;
					}

					return idx + (tile.idx & 0x7F);
				}();

				auto pTile = ppu.getTile(meta.tiles[tileIdx].src);
				for (u8 c = 0; c < 8; ++c) {
					if (!pTile) {
						bg.push({.valid = false});
					}
					else {
						bg.push({.col = 0xFFFFFF});
					}
				}
			}

			/*
			const Color pixel = [&, &pixel = buffer.pixels[y * 160 + p]]() {
				// todo: either clear the screen when bios -> game or use the bios as a fallback
				
				auto pTile = ppu.getTile(meta.tiles[bgPixel.idx].src);
				if (!pTile) {
					return pixel.getColor(ppu.model);
				}
				
				auto& tile = *pTile;
				auto col = tile.data[bgPixel.x + (bgPixel.y * 8)];
				if (tile.usesIndexColors) {
					for (u8 i = 0; i < indexColors.size(); ++i) {
						if (indexColors[i] == col) {
							if (context.gb.cgbMode) {
								return PPU::cgbPaletteColors[meta.palettes[bgPixel.pal].cgb[i]];
							}
							else {
								return PPU::paletteColors[meta.palettes[bgPixel.pal].dmg[i]];
							}
						}
					}
				}
				return col;
			}();
			*/
			size_t idx = y * 160 * 4 + p * 4;
			auto bgPixel = bg.pop();
			auto pixel = (bgPixel.valid) ? bgPixel.col : buffer.pixels[y * 160 + p].getColor(ppu.model);
			display.data()[idx] = pixel.r;
			display.data()[idx + 1] = pixel.g;
			display.data()[idx + 2] = pixel.b;
			display.data()[idx + 3] = pixel.a;
		}
	}
}

void DisplayWindow::renderScene(Framebuffer& buffer) {
	for (size_t y = 0; y < 144; ++y) {
		for (size_t p = 0; p < 160; ++p) {
			u8 color = buffer.pixels[y * 160 + p].color;

			u8 back = 1;
			u8 front = (color != 0) ? color + 1 : 0;
			
			//scene.screen.models[0].volume[(d * 160 * 144) (143 - y) * 160 + p] = back;
			for (u8 d = 0; d < 8; ++d) 
				scene.screen.models[0].volume[(d * 160 * 144) + (143 - y) * 160 + p] = front;
		}
	}
	scene.screen.models[0].update();
}

void DisplayWindow::captureMouse(bool hasMouse) {
	if (hasMouse) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
		avoidReset = true;
	}
	else {
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}

	SDL_SetWindowRelativeMouseMode(SDL_GL_GetCurrentWindow(), hasMouse);
	inFocus = hasMouse;
}

} // namespace ui 
