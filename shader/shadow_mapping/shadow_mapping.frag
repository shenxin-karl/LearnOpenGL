#version 330 core

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
	vec4	light_space_position;
} fs_in;

uniform sampler2D diffuse_map1;
uniform sampler2D depth_map;
uniform vec3 eye_pos;
uniform vec3 light_dir;

out vec4 frag_color;

float shadow_mapping(vec4 light_space_position, vec3 normal) {
	vec3 ndc = light_space_position.xyz / light_space_position.w;
	ndc = ndc * 0.5 + 0.5;
	if (ndc.z > 1) 
		return 0;

	const int limit = 1;
	float bias = mix(0.01, 0.005, max(dot(normal, light_dir), 0));
	float curr_depth = ndc.z - bias;
	float shadow = 0.0;
	vec2 tex_size = 1.0 / textureSize(depth_map, 0);
	for (int x = -limit; x <= limit; ++x) {
		for (int y = -limit; y <= limit; ++y) {
			vec2 offset = vec2(x, y) * tex_size;
			float pcf_depth = texture(depth_map, ndc.xy + offset).r;
			shadow += curr_depth <= pcf_depth ? 1 : 0;
		}
	}
	shadow /= (limit * 9);
	return shadow;
}

void main() {
	vec3 normal = normalize(fs_in.normal);
	vec3 view_dir = normalize(eye_pos - fs_in.position);
	vec3 diffuse_color = texture(diffuse_map1, fs_in.texcoord).rgb;
	float shadow = shadow_mapping(fs_in.light_space_position, normal);

	// ambient
	vec3 ambinet = diffuse_color * 0.2;

	// diffuse
	float diff = max(dot(light_dir, normal), 0.0);
	vec3 diffuse = (diff * 0.6 * shadow) * diffuse_color;

	vec3 half_vec = normalize(view_dir + light_dir);
	float spec = pow(max(dot(half_vec, normal), 0.0), 64.0);
	vec3 specular = (spec * 0.2 * shadow) * diffuse_color;

	const float gamma = 2.2;
	vec3 color = ambinet + diffuse + specular;
	color = pow(color, vec3(1 / gamma));
	frag_color = vec4(color, 1.0);
}
