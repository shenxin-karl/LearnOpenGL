#version 330 core
layout(location = 0) out vec3 g_position;
layout(location = 1) out vec3 g_normal;
layout(location = 2) out vec3 g_color;

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
} fs_in;

uniform sampler2D diffuse_map;

void main() {
	g_position = fs_in.position;
	g_normal   = normalize(fs_in.normal);
	g_color    = texture(diffuse_map, fs_in.texcoord).rgb;
}