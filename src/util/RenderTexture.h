#pragma once

#include "gl/Texture.h"

#include <functional>

#include "ImGuiHeaders.h"
#include "Types.h"

class RenderTexture : public Texture {
	GLsizei width, height;
	std::string name;

public:
	u32* data;

	RenderTexture(GLsizei width_, GLsizei height_, u32* data_, const char* name_ = "") : Texture(GL_REPEAT, GL_NEAREST) {
		width = width_;
		height = height_;
		data = data_;
		name = name_;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, data);
	}

	void update() {
		glBindTexture(GL_TEXTURE_2D, id);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, data);
	}

	//pass this in to draw extra stuff
	using DrawCallback = std::function<void(const ImVec2& topLeft, const ImVec2& bottomRight, float zoomMult)>;

	void render(float zoom_mult, bool grid, DrawCallback extraCallback = nullptr);

private:
	void* _getTextureId() {
		return reinterpret_cast<ImTextureID>(static_cast<intptr_t>(id));
	}
};