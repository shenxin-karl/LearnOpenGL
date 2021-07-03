#version 330 core

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 bright_color;
uniform vec3 light_color;

void main() {
/*
	// HDR
	vec3 color = light_color;
	color = color / (color + vec3(1.0));

	const float gamma = 2.2;
	color = pow(color, vec3(1.0 / gamma));
*/
	frag_color = vec4(light_color, 1.0);
	bright_color = frag_color;
}