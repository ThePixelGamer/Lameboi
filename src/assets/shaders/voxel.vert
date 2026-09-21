#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 invModel;
uniform mat4 mvp;

uniform vec3 camera;

out vec3 fV;
out vec3 pos;

void main() {
    pos = aPos;
    vec3 world_pos = (model * vec4(aPos, 1)).xyz;

    fV = (invModel * vec4(world_pos - camera, 0)).xyz;

	gl_Position = mvp * vec4(aPos, 1.0);
}
