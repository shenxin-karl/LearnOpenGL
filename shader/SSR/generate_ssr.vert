#version 330 core
layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texcoord;

out vec2 our_texcoord;

void main() {
	gl_Position  = vec4(position, 1.0);
	our_texcoord = texcoord;
}