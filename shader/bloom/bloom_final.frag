#version 330 core

out vec4 frag_color;
in vec2 our_texcoord;

uniform sampler2D color_map;
uniform sampler2D birght_color_map;

void main() {
	vec3 color1 = texture(color_map, our_texcoord).rgb;
	vec3 color2 = texture(birght_color_map, our_texcoord).rgb;

	vec3 color = color1 + color2;
	// vec3 color = color1;
	color = color / (color + vec3(1));

	const float gamma = 2.2;
	color = pow(color, vec3(1 / gamma));
	frag_color = vec4(color, 1.0);
/*
	frag_color = vec4(our_texcoord, 0.0, 0.0);
*/
}