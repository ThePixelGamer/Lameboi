#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D boxTexture;
uniform sampler2D faceTexture;

void main() {
   FragColor = mix(texture(boxTexture, TexCoord), texture(faceTexture, TexCoord), 0.2);
}