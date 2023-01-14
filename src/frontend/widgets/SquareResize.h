#pragma once

#include <algorithm>

#include "util/ImGuiHeaders.h"

inline void squareResize(ImGuiSizeCallbackData* data) {
	auto beforeImage = static_cast<ImVec2*>(data->UserData);
	ImGuiContext& g = *GImGui;
	ImVec2& padding = ImGui::GetStyle().WindowPadding;
	ImVec2 size = data->DesiredSize - *beforeImage - padding;

	float m = std::max(size.x, size.y);
	switch (g.MouseCursor) {
		case ImGuiMouseCursor_ResizeEW:
			m = size.x;
			break;

		case ImGuiMouseCursor_ResizeNS:
			m = size.y;
			break;

		default: break; // shouldn't happen?
	}

	data->DesiredSize.x = m + beforeImage->x + padding.x;
	data->DesiredSize.y = m + beforeImage->y + padding.y;
}