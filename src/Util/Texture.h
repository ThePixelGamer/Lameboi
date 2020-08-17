#pragma once

#include "ImGuiHeaders.h"
#include "Types.h"

struct Texture {
	GLuint texture;
	GLsizei width, height;
	u32* data;
	const char* name;
};

Texture genTexture(GLsizei width, GLsizei height, u32* pixels, const char* name) {
	Texture tex{ 0, width, height, pixels, name };
	glGenTextures(1, &tex.texture);
	glBindTexture(GL_TEXTURE_2D, tex.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	return tex;
}

void updateTextures(Texture& tex) {
	glBindTexture(GL_TEXTURE_2D, tex.texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex.width, tex.height, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tex.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}