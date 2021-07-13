#version 330 core
in vec3 our_texcoord;
out vec4 frag_color;
uniform samplerCube env_cube_map;
uniform float roughness;

void main() {
	// vec3 color = texture(env_cube_map, our_texcoord).rgb;
	vec3 color = textureLod(env_cube_map, our_texcoord, roughness).rgb;
	color      = pow(color, vec3(1/2.2));
	frag_color = vec4(color, 1.0);
	// frag_color = vec4(normalize(our_texcoord) * 0.5 + 0.5, 1.0);
}