#include "RenderTexture.h"

#include <algorithm>

void RenderTexture::render(float zoom_mult) {
	float adjWidth = width * v.x * zoom_mult;
	float adjHeight = height * v.y * zoom_mult;

	ImGui::Image(_getTextureId(), ImVec2(adjWidth, adjHeight), u, v);
}

std::tuple<bool, u32, u32> RenderTexture::render(float zoom_mult, bool grid, DrawCallback extraCallback, void* extraData) {
	ImGuiIO& io = ImGui::GetIO();
	float adjWidth = width * zoom_mult;
	float adjHeight = height * zoom_mult;

	bool clicked = ImGui::ImageButton(_getTextureId(), ImVec2(adjWidth, adjHeight), u, v, 0);

	ImVec2 topleft = ImGui::GetItemRectMin();
	ImVec2 bottomright = ImGui::GetItemRectMax();

	float region_sz = 8.0f * zoom_mult; //region = 8x8 area
	u32 region_x = std::clamp((io.MousePos.x - topleft.x) / region_sz, 0.0f, adjWidth);
	u32 region_y = std::clamp((io.MousePos.y - topleft.y) / region_sz, 0.0f, adjHeight);

	if (grid) {
		float line_dist = 8.0f * zoom_mult;

		for (float x = topleft.x + line_dist; x <= bottomright.x - line_dist; x += line_dist) {
			ImGui::GetWindowDrawList()->AddLine(ImVec2(x, topleft.y), ImVec2(x, bottomright.y), IM_COL32(169, 169, 169, 255));
		}

		for (float y = topleft.y + line_dist; y <= bottomright.y - line_dist; y += line_dist) {
			ImGui::GetWindowDrawList()->AddLine(ImVec2(topleft.x, y), ImVec2(bottomright.x, y), IM_COL32(169, 169, 169, 255));
		}

		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();

			constexpr float zoom = 4.0f;

			ImVec2 uv0 = ImVec2((region_x * region_sz) / adjWidth, (region_y * region_sz) / adjHeight);
			ImVec2 uv1 = ImVec2(((region_x + 1.0f) * region_sz) / adjWidth, ((region_y + 1.0f) * region_sz) / adjHeight);

			ImGui::Text("Coordinate: (%d, %d)", region_x, region_y);
			ImGui::Image(_getTextureId(), ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1);

			ImGui::EndTooltip();
		}
	}

	if (extraCallback) {
		extraCallback(extraData, topleft, bottomright, zoom_mult);
	}

	return { clicked, region_x, region_y };
}

void RenderTexture::setSize(float w, float h) {
	v.x = w / width;
	v.y = h / height;
}
