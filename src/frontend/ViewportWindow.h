#pragma once

#include "util/ImGuiHeaders.h"
#include "util/GLHeaders.h"

#include <array>
#include <fstream>

#include <glm/glm.hpp>

#include "util/gl/ImageTexture.h"
#include "util/gl/Shader.h"

namespace ui {

class ViewportWindow {
	Shader shader;
	GLuint VAO;
	GLuint framebuffer, textureColorbuffer;
	std::unique_ptr<ImageTexture> boxTexture, faceTexture;

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	
	const std::array<glm::vec3, 10> cubePos {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	bool wireframe = false;

	const GLsizei internalWidth = 1080;
	const GLsizei internalHeight = 1080;

	bool inFocus = false;
	bool avoidReset = false;
	float yaw = -90.0f, pitch = 0.0f;
	float lastX = internalWidth / 2.0f, lastY = internalHeight / 2.0f;
	float fov = 70.0f;

public:
	bool show = false;

	ViewportWindow();

	void render();

	// todo: add input handlers for camera controls (mouse support?)
	void registerInputHandlers() {

	}

private:
	void drawOptions();
};

} // namespace ui