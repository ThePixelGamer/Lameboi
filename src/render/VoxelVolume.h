#pragma once

#include <filesystem>
#include <array>
#include <glm/ext/matrix_transform.hpp>
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>

#include "Cube.h"
#include "Shader.h"
#include "Texture.h"
#include "util/Types.h"

class VoxelTexture : public Texture {
private:
	constexpr static GLenum Target = GL_TEXTURE_3D;

public:
	GLsizei W, H, D;

	VoxelTexture(GLsizei w, GLsizei h, GLsizei d) : W(w), H(h), D(d) {
		init();
	}

	void use() {
		glBindTexture(Target, id);
	}

	void init() {
		use();

		glTexParameteri(Target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(Target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(Target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexStorage3D(Target, 1, GL_R8, W, H, D);
		
		//glTexImage3D(Target, 0, GL_RED, W, H, D, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	}
	
	void update(void* data) {
		use();
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage3D(Target, 0, 0, 0, 0, W, H, D, GL_RED, GL_UNSIGNED_BYTE, data);
	}
};

class VoxelVolume {
	GLuint VAO, VBO, EBO;
	Shader shader{"shaders/voxel.vert", "shaders/voxel.frag"};

	struct Model {
		std::vector<u8> volume;
		VoxelTexture texture;

		Model(u32 w, u32 h, u32 d) : texture(w, h, d) {
			volume.resize(w * h * d);
		}
	};
	
	std::vector<Model> models;
	std::array<glm::vec3, 256> palette;

	using clock = std::chrono::high_resolution_clock;
	clock::time_point perfTimer = clock::now();

public:
	VoxelVolume(std::filesystem::path path) {
		load(path);
		init();
	}

	void load(std::filesystem::path path);

	void init() {
		GLfloat vertices[] = {
			0, 1, 1,    // 0 -x face provoking vertex
            0, 0, 1,    // 1 +z face provoking vertex
            1, 0, 1,    // 2 +x face provoking vertex
            1, 1, 1,    // 3 unused as provoking vertex
            0, 1, 0,    // 4 +y face provoking vertex
            0, 0, 0,    // 5 -y face provoking vertex
            1, 0, 0,    // 6 unused as provoking vertex
            1, 1, 0
    	};

    	GLuint indices[] = {
            6, 7, 2, 7, 3, 2,   // +x face
            4, 5, 0, 5, 1, 0,   // -x face
            0, 3, 4, 3, 7, 4,   // +y face
            6, 2, 5, 2, 1, 5,   // -y face
            2, 3, 1, 3, 0, 1,   // +z face
            6, 5, 7, 5, 4, 7,   // -z face
    	};
		
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		
		// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
		// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
		glBindVertexArray(0);
	}

	void render(glm::vec3& cameraPos, glm::vec3 translate, glm::mat4& view, glm::mat4& projection) {
		static u8 i = 0;
		glActiveTexture(GL_TEXTURE0);
		using namespace std::chrono_literals;
		auto now = clock::now();
		auto perf = std::chrono::duration_cast<std::chrono::seconds>(clock::now() - perfTimer);
		if (perf >= 1s) {
			perfTimer = now;
			//i = (i == models.size() - 1) ? 0 : i + 1;
		}
		models[i].texture.use();

		shader.use();

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);   
		
		glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

		float test = std::max(models[i].texture.W, std::max(models[i].texture.H, models[i].texture.D));
		glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(models[i].texture.W / test, models[i].texture.H / test, models[i].texture.D / test));
		glm::mat4 inverseModel = glm::inverse(model);
		model = glm::translate(model, translate);

		shader.setVec3("model_cam_pos", inverseModel * glm::vec4(cameraPos, 1.0f));
    	
		shader.setVec3("camera", cameraPos);
		shader.setMat4("mvp", projection * view * model);
		shader.setMat4("invModel", inverseModel);
		shader.setMat4("model", model);
		
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glDisable(GL_CULL_FACE);
	}
};
