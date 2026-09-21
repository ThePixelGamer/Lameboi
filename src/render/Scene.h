#pragma once

#include <array>
#include <memory>

#include "Texture.h"
#include "Shader.h"
#include "VoxelVolume.h"
#include "Cubemap.h"

class Scene {
private:
	GLuint framebuffer, textureColorbuffer;
	ImageTexture boxTexture;
	VoxelVolume deer { "deer.vox"};
	//VoxelVolume trex { "T-Rex.vox"};
	//VoxelVolume horse { "horse.vox"};
	Cubemap cubemap;
	TexturedCube cube;
	Shader shader{"shaders/cube.vert", "shaders/cube.frag"};

	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

	float yaw = -90.0f, pitch = 0.0f;

public:
	const GLsizei Width = 1080, Height = 1080;

	bool wireframe = false;
	float fov = 70.0f;

	Scene();

	GLuint render();
	bool handleInput(bool& avoidReset);
};
