#pragma once

#include <functional>

#include "ImGuiHeaders.h"
#include "Types.h"

class Texture {
	GLuint mId = 0;
	GLsizei mWidth, mHeight;
	std::string mName;

public:
	u32* mData;

	Texture(GLsizei width, GLsizei height, u32* pixels, const char* name = "") :
		mWidth(width),
		mHeight(height),
		mData(pixels),
		mName(name)
	{
		glGenTextures(1, &mId);
		glBindTexture(GL_TEXTURE_2D, mId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mWidth, mHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, mData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	void update() {
		glBindTexture(GL_TEXTURE_2D, mId);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mWidth, mHeight, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, mData);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	//pass this in to draw extra stuff
	using DrawCallback = std::function<void(const ImVec2& topLeft, const ImVec2& bottomRight, u32 zoomMult)>;

	void display(u32 zoom_mult, bool grid, DrawCallback extraCallback = nullptr) {
		ImGuiIO& io = ImGui::GetIO(); 
		u32 width = mWidth * zoom_mult;
		u32 height = mHeight * zoom_mult;

		if (!mName.empty())
			ImGui::Text((mName + ": %dx%d").c_str(), width, height);
		
		ImGui::Image(_getTextureId(), ImVec2(float(width), float(height)), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

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

			if (extraCallback) {
				extraCallback(topleft, bottomright, zoom_mult);
			}

			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();

				constexpr float zoom = 4.0f;
				float region_sz = 8.0f * zoom_mult; //region = 8x8 area

				u32 region_x = std::clamp(u32((io.MousePos.x - topleft.x) / region_sz), 0u, width);
				u32 region_y = std::clamp(u32((io.MousePos.y - topleft.y) / region_sz), 0u, height);
				ImVec2 uv0 = ImVec2((region_x * region_sz) / width, (region_y * region_sz) / height);
				ImVec2 uv1 = ImVec2(((region_x + 1) * region_sz) / width, ((region_y + 1) * region_sz) / height);

				ImGui::Text("Coordinate: (%d, %d)", region_x + 1, region_y + 1);
				ImGui::Image(_getTextureId(), ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1);

				ImGui::EndTooltip();
			}
		}
	}

private:

	void* _getTextureId() {
		return reinterpret_cast<void*>(static_cast<intptr_t>(mId));
	}
};