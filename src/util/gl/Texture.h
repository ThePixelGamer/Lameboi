#pragma once

#include "util/GLHeaders.h"

class Texture {
protected:
	const GLint WrapValue;
	const GLint FilterValue;

public:
	GLuint id = 0;

	Texture(GLint wrap, GLint filter) : WrapValue(wrap), FilterValue(filter) {
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapValue);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapValue);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterValue);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterValue);
	}
};