#pragma once

#include "gl/Texture.h"

#include <functional>

#include "ImGuiHeaders.h"
#include "Types.h"

class RenderTexture : public Texture {
	GLsizei width, height;

	ImVec2 u{ 0, 0 }, v{ 1, 1 };

public:
	u32* data;

	RenderTexture(GLsizei width_, GLsizei height_, u32* data_) : Texture(GL_REPEAT, GL_NEAREST) {
		width = width_;
		height = height_;
		data = data_;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, data);
	}

	void update() {
		glBindTexture(GL_TEXTURE_2D, id);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, data);
	}

	//pass this in to draw extra stuff
	using DrawCallback = std::function<void(void* data, const ImVec2& topLeft, const ImVec2& bottomRight, float zoomMult)>;

	void render(float zoom_mult);
	std::tuple<bool, u32, u32> render(float zoom_mult, bool grid, DrawCallback extraCallback = nullptr, void* extraData = nullptr);

	void setSize(float w, float h);

private:
	void* _getTextureId() {
		return reinterpret_cast<ImTextureID>(static_cast<intptr_t>(id));
	}
};