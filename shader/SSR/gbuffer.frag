#version 330 core
layout(location = 0) out vec4 g_position;
layout(location = 1) out vec3 g_normal;
layout(location = 2) out vec4 g_albedo;

uniform float NEAR;
uniform float FAR;
uniform sampler2D diffuse_map;
uniform sampler2D roughness_map;

in VS_OUT {
	vec4	position;
	vec2	texcoord;
	vec3	normal;
} fs_in;

float linear_depth() {
	float z = gl_FragCoord.z * 2.0 - 1.0;
	return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));
}

void main() {
	g_position = vec4(fs_in.position.xyz, linear_depth());
	g_normal   = normalize(fs_in.normal);
	g_albedo   = vec4(texture(diffuse_map, fs_in.texcoord).rgb, texture(roughness_map, fs_in.texcoord).r);
}