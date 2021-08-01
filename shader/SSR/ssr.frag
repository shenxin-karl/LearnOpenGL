#version 330 core
out vec4 frag_color;

in vec2 our_texcoord;
uniform sampler2D position_buffer;

void main() {
	vec3 color = texture(position_buffer, our_texcoord).xyz;
	color = normalize(color);
	frag_color = vec4(color, 1.0);
}	