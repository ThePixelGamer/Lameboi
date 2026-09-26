#include "Scene.h"

#include <SDL3/SDL_mouse.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL3/SDL.h>
#include <stb_image.h>

Scene::Scene() : boxTexture("box.jpg") {
	// setup framebuffer
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// create a color attachment texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1080, 1080, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1080, 1080); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LB_ERROR(Frontend, "FRAMEBUFFER::Framebuffer is not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint Scene::render(GLsizei W, GLsizei H) {
	if (W != Width || H != Height) {
		Width = W;
		Height = H;

		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, W, H, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, W, H);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	glPolygonMode(GL_FRONT_AND_BACK, (wireframe) ? GL_LINE : GL_FILL);

	// bind to framebuffer and draw scene as we normally would to color texture 
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

	// make sure we clear the framebuffer's content
	glViewport(0, 0, Width, Height);
	glClearColor(0xfa / 255.0f, 0xfb / 255.0f, 0xf6 / 255.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::vec3 direction;
	direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
	direction.y = std::sin(glm::radians(pitch));
	direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);

	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)Width / (float)Height, 0.1f, 100.0f);

	screen.render(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), view, projection);
	//deer.render(cameraPos, glm::vec3(1.0f, 0.0f, 0.0f), view, projection);

	//trex.render(cameraPos, glm::vec3(2.0f, 0.0f, 0.0f), view, projection);
	//horse.render(cameraPos, glm::vec3(3.0f, 0.0f, 0.0f), view, projection);
	//cubemap.render(view, projection);

	// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

	return textureColorbuffer;
}

Scene::InputAction Scene::handleInput(bool& avoidReset) {
	const bool* state = SDL_GetKeyboardState(NULL);
	const float cameraSpeed = 0.05f; // adjust accordingly
	if (state[SDL_SCANCODE_T])
		cameraPos += cameraSpeed * cameraFront;
	if (state[SDL_SCANCODE_G])
		cameraPos -= cameraSpeed * cameraFront;
	if (state[SDL_SCANCODE_F])
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (state[SDL_SCANCODE_H])
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

	if (state[SDL_SCANCODE_ESCAPE]) {
		return UnlockMouse;
	}

	float xpos = 0, ypos = 0;
	SDL_MouseButtonFlags mouse = SDL_GetRelativeMouseState(&xpos, &ypos);

	if (mouse & SDL_BUTTON_RMASK) {
		return OpenContextMenu;
	}

	if (avoidReset) {
		avoidReset = false;
	}
	else {
		const float sensitivity = 0.1f;
		yaw += xpos * sensitivity;
		pitch += -ypos * sensitivity;

		const float maxAngle = 89.0f;
		if (pitch > maxAngle)
			pitch = maxAngle;
		if (pitch < -maxAngle)
			pitch = -maxAngle;
	}

	return None;
}
