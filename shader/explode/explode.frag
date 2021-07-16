#version 330 core

out vec4 frag_color;

in GEOM_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
} fs_in;

uniform sampler2D diffuse_map;
uniform vec3 view_pos;
uniform vec3 light_pos;
uniform vec3 light_color;

void main() {
	vec3 albedo = texture(diffuse_map, fs_in.texcoord).rgb;
	vec3 N = normalize(fs_in.normal);
	vec3 V = normalize(view_pos - fs_in.position);
	vec3 L = normalize(light_pos - fs_in.position);
	vec3 H = normalize(V + L);

	vec3 ambient  = vec3(0.1) * albedo;
	vec3 diffuse  = 0.7 * max(dot(N, L), 0.0) * albedo * light_color;
	vec3 specular = 0.2 * pow(max(dot(H, N), 0.0), 64.0) * albedo * light_color;

	vec3 color = ambient + diffuse + specular;
	frag_color = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}
