#version 330 core

out vec4 frag_color;

in VS_OUT {
	vec2 texcoord;
} fs_in;

uniform sampler2D diffuse_map;

void main() {
	vec3 color = texture(diffuse_map, fs_in.texcoord).rgb;
	color      = pow(color, vec3(1.0 / 2.2));
	frag_color = vec4(color, 1.0);
}