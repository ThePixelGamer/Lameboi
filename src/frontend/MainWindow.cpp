#include "MainWindow.h"

#include <imgui_internal.h>

bool UI::render() {
	auto flags = ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_PassthruCentralNode;
	auto dockid = ImGui::DockSpaceOverViewport(ImGui::GetID("lameboi"), nullptr, flags);
	auto centralNode = ImGui::DockBuilderGetCentralNode(dockid);
	centralNode->SetLocalFlags(centralNode->LocalFlags | ImGuiDockNodeFlags_NoUndocking | ImGuiDockNodeFlags_NoTabBar);
	ImGui::SetNextWindowDockID(centralNode->ID, ImGuiCond_Once);
	
	display.render();
	menubar.render();

	// Display Formats (2D screen or voxel rendering)
	viewport.render();

	// Gameboy Debug Stuff
	debug.render();
	settings.render();
	bgmapWindow.render();
	tileDataWindow.render();
	oamWindow.render();
	apuWindow.render();

	return requestExit;
}
