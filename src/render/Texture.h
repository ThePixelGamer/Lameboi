#pragma once

#include <glad/glad.h>

#include <util/Log.h>

class Texture {
protected:
	GLuint id = 0;

public:
	Texture() {
		glGenTextures(1, &id);
	}

	~Texture() {
		glDeleteTextures(1, &id);
	}
};

class ImageTexture : public Texture {
public:
	constexpr static GLenum Target = GL_TEXTURE_2D;
	constexpr static GLint InternalFormat = GL_RGBA8;
	constexpr static GLenum Type = GL_UNSIGNED_BYTE;

	GLenum Format = GL_RGBA;
	const GLint FilterValue = GL_NEAREST;

private:
	const GLint WrapValue = GL_REPEAT;

	GLsizei W, H;
	void* data = nullptr;

public:
	ImageTexture(GLsizei w, GLsizei h, void* data) : W(w), H(h), data(data) {
		init();
		update(data);
	}

	ImageTexture(const char* file);

	void use() {
		glBindTexture(Target, id);
	}

	void init() {
		use();

		glTexParameteri(Target, GL_TEXTURE_WRAP_S, WrapValue);
		glTexParameteri(Target, GL_TEXTURE_WRAP_T, WrapValue);
		glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, FilterValue);
		glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, FilterValue);

		glTexStorage2D(Target, 1, InternalFormat, W, H);
	}

	void update(void* user_data = nullptr) {
		use();
		glTexSubImage2D(Target, 0, 0, 0, W, H, Format, Type, (user_data) ? user_data : data);

	}

	template <typename T>
	T as() {
		return static_cast<T>(id);
	}
};