#include "ImageTexture.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif

#include "stb_image.h"

#include "util/GLHeaders.h"
#include "util/Log.h"

ImageTexture::ImageTexture(const char* path) : Texture(GL_REPEAT, GL_LINEAR) {
	// load image, create texture and generate mipmaps
	int width, height, nrComponents;
	auto data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		auto [format, internalFormat] = [&]() -> std::pair<GLenum, GLint> {
			switch (nrComponents) {
				case STBI_rgb: return { GL_RGB, GL_RGB8 };
				case STBI_rgb_alpha: return { GL_RGBA, GL_RGBA8 };

				default:
					LB_WARN(Util, "Unhandled STBI Component {}", nrComponents);
					return { GL_RED, GL_R8 };
			}
		}();

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		LB_WARN(Util, "Failed to load texture");
	}
	stbi_image_free(data);
}