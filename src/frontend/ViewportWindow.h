#pragma once

#include <imgui.h>
#include <SDL3/SDL.h>

#include "render/Scene.h"

namespace ui {

class ViewportWindow {
private:
	Scene scene;
	bool inFocus = false; 
	bool avoidReset = false;

public:
	bool show = false;

	ViewportWindow() = default;

	// capturing the mouse seems to be broken
	void render() {
		if (inFocus && !scene.handleInput(avoidReset)) {
			captureMouse(false);
		}

		if (show) {
			ImGui::Begin("##Viewport", &show, ImGuiWindowFlags_AlwaysAutoResize);

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
			if (ImGui::ImageButton("##viewport", (ImTextureID)scene.render(), ImVec2(scene.Width / 2.0f, scene.Height / 2.0f), ImVec2(0, 1), ImVec2(1, 0))) {
				captureMouse(true);
			}
			ImGui::PopStyleVar();

			drawOptions();

			ImGui::End();
		}
	}

private:
	void captureMouse(bool hasMouse) {
		if (hasMouse) {
			ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
			avoidReset = true;
		}
		else {
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
		}

		SDL_SetWindowRelativeMouseMode(NULL, hasMouse);
		inFocus = hasMouse;
	}

	void drawOptions() {
		if (ImGui::Button("Options")) {
			ImGui::OpenPopup("##options_context");
		}

		if (ImGui::BeginPopup("##options_context")) {
			ImGui::Checkbox("Wireframe", &scene.wireframe);
			ImGui::SliderFloat("FOV", &scene.fov, 30.0f, 150.0f, "%.0f");

			ImGui::EndPopup();
		}
	}
};

} // namespace ui