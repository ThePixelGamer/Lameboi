#pragma once

#include "Cube.h"
#include "Shader.h"
#include "Texture.h"

class Cubemap {
	Cube cube;
	Shader shader{"shaders/skybox.vert", "shaders/skybox.frag"};
	CubemapTexture texture{"skybox/"};

public:
	void render(glm::mat4 view, glm::mat4& projection) {
        glDepthFunc(GL_LEQUAL); 
		shader.use();
        // remove translation from the view matrix
        shader.setMat4("view", glm::mat4(glm::mat3(view)));
        shader.setMat4("projection", projection);

        glBindVertexArray(cube.VAO);
        glActiveTexture(GL_TEXTURE0);
		texture.use();
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
	}
};
