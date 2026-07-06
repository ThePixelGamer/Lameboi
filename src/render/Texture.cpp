#include "Texture.h"

#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "util/Log.h"

ImageTexture::ImageTexture(const char* path) : FilterValue(GL_LINEAR) {
	// flip loaded texture's on the y-axis
	stbi_set_flip_vertically_on_load(true); 

	// load image, create texture and generate mipmaps
	int nrComponents;
	stbi_uc* img = stbi_load(path, &W, &H, &nrComponents, 0);
	if (!img) {
		LB_WARN(Util, "Failed to load ImageTexture {}", path);
		return;
	}

	init();

	switch (nrComponents) {
		case STBI_rgb: 
			Format = GL_RGB;
			break;

		case STBI_rgb_alpha:
			Format = GL_RGBA;
			break;

		default:
			LB_WARN(Util, "Unhandled STBI Component {}", nrComponents);
			break;
	}

	// avoid storing the data so we can free it
	update(img);
	stbi_image_free(img);
}