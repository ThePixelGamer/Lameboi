#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 vp;
uniform vec3 camera;
uniform mat4 model;

flat out vec3 model_cam_pos;
out vec3 fV;
out vec3 pos;

void main() {
    pos = aPos;
    vec3 world_pos = (model * vec4(aPos, 1)).xyz;
	mat4 invModel = inverse(model);
    fV = (invModel * vec4(world_pos - camera, 0)).xyz;
	model_cam_pos = (invModel * vec4(camera, 1)).xyz;

	gl_Position = vp * model * vec4(aPos, 1.0);
}
