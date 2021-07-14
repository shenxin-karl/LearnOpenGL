#version 330 core
in vec2 our_texcoord;
out vec4 frag_color;
uniform sampler2D texture_map;

void main() {
	vec3 color = texture(texture_map, our_texcoord).rgb;
	color      = color / (1.0 + color);
	color      = pow(color, vec3(1.0 / 2.2));
	frag_color = vec4(color, 1.0);
}