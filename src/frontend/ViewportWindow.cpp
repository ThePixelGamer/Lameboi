#include "ViewportWindow.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>

#include "util/SDLInterface.h"

namespace ui {

ViewportWindow::ViewportWindow() : shader("shaders/cube.vert", "shaders/cube.frag") {
	registerInputHandlers();

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	GLuint VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

	boxTexture = std::make_unique<ImageTexture>("D:/Pixel/Pictures/Dev Stuff/container.jpg");
	faceTexture = std::make_unique<ImageTexture>("D:/Pixel/Pictures/Dev Stuff/awesomeface.png");

	shader.use();
	shader.setInt("boxTexture", 0);
	shader.setInt("faceTexture", 1);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// setup framebuffer
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// create a color attachment texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, internalWidth, internalHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, internalWidth, internalHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LB_ERROR(Frontend, "FRAMEBUFFER::Framebuffer is not complete!");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewportWindow::drawOptions() {
	if (ImGui::Button("Options")) {
		ImGui::OpenPopup("##options_context");
	}

	if (ImGui::BeginPopup("##options_context")) {
		ImGui::Checkbox("Wireframe", &wireframe);
		ImGui::SliderFloat("FOV", &fov, 30.0f, 150.0f, "%.0f");

		ImGui::EndPopup();
	}
}

void ViewportWindow::render() {
	if (!show) {
		return;
	}
 
	float footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

	ImGui::Begin("##Viewport", &show, ImGuiWindowFlags_AlwaysAutoResize);

	glPolygonMode(GL_FRONT_AND_BACK, (wireframe) ? GL_LINE : GL_FILL);

	// bind to framebuffer and draw scene as we normally would to color texture 
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)

	// make sure we clear the framebuffer's content
	glViewport(0, 0, internalWidth, internalHeight);
	//glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, boxTexture->id);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, faceTexture->id);

	shader.use();

	if (inFocus) {
		const Uint8* state = SDL_GetKeyboardState(NULL);
		const float cameraSpeed = 0.05f; // adjust accordingly
		if (state[SDL_SCANCODE_W] == SDL_PRESSED)
			cameraPos += cameraSpeed * cameraFront;
		if (state[SDL_SCANCODE_S] == SDL_PRESSED)
			cameraPos -= cameraSpeed * cameraFront;
		if (state[SDL_SCANCODE_A] == SDL_PRESSED)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (state[SDL_SCANCODE_D] == SDL_PRESSED)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

		if (state[SDL_SCANCODE_ESCAPE] == SDL_PRESSED) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
			inFocus = false;
		}
		else {
			int xpos_ = 0, ypos_ = 0;
			SDL_GetRelativeMouseState(&xpos_, &ypos_);
			float xpos = (float)xpos_;
			float ypos = (float)ypos_;

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

			/*
			if (xpos != lastX || ypos != lastY) {
				float xoffset = xpos - lastX;
				float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
				lastX = xpos;
				lastY = ypos;

				const float sensitivity = 0.1f;
				yaw += xoffset * sensitivity;
				pitch += yoffset * sensitivity;

				const float maxAngle = 89.0f;
				if (pitch > maxAngle)
					pitch = maxAngle;
				if (pitch < -maxAngle)
					pitch = -maxAngle;
			}
			*/
		}
	}

	glm::vec3 direction;
	direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
	direction.y = std::sin(glm::radians(pitch));
	direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);

	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	shader.setMat4("view", view);

	glm::mat4 projection = glm::perspective(glm::radians(fov), (float)internalWidth / (float)internalHeight, 0.1f, 100.0f);
	shader.setMat4("projection", projection);

	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	for (size_t i = 0; i < cubePos.size(); ++i) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, cubePos[i]);
		model = glm::rotate(model, glm::radians(20.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
		shader.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	glBindVertexArray(0);

	// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

	if (ImGui::ImageButton((ImTextureID)textureColorbuffer, ImVec2(internalWidth / 2.0f, internalHeight / 2.0f), ImVec2(0, 1), ImVec2(1, 0), 0)) {
		SDL_SetRelativeMouseMode(SDL_TRUE);
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
		avoidReset = true;
		inFocus = true;
	}

	drawOptions();

	ImGui::End();
}

} // namespace ui