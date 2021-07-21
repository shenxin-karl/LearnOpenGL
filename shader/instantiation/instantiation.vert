#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 offset;

out vec3 our_color;

void main() {
	gl_Position = vec4(position + offset, 0.0, 1.0);
	our_color   = color;
}
