#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D boxTexture;
uniform sampler2D faceTexture;

void main() {
   FragColor = texture(boxTexture, TexCoord);
}
