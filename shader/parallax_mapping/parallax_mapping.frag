#version 330 core

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	tangent_light_dir;
	vec3	tangent_view_pos;
	vec3	tangent_position;
} fs_in;

uniform sampler2D diffuse_map1;
uniform sampler2D normal_map1;
uniform sampler2D displacement_map1;
uniform float displacement_scale;

out vec4 frag_color;

vec2 parallax_mapping(sampler2D depth_map, vec2 texcoord, vec3 view_dir) {
	float depth = texture(depth_map, texcoord).r;
	vec2 p = view_dir.xy / view_dir.z * (depth * displacement_scale);
	return texcoord - p;
}


vec2 parallax_mapping1(sampler2D depth_map, vec2 texcoord, vec3 view_dir) {
	float depth = texture(depth_map, texcoord).r;
	vec2 p = view_dir.xy / view_dir.z * (depth * displacement_scale);
	const float min_layers = 8;
	const float max_layers = 32;
	float layers = mix(min_layers, max_layers, abs(view_dir.z));
	float delta_depth = 1.f / layers;
	vec2 delta_p = p / layers;
	float curr_depth = 1.0;
	float expect_depth = 0.0;

	while (curr_depth > expect_depth) {		// find the first curr_depth less expect_depth
		expect_depth += delta_depth;
		texcoord -= delta_p;
		curr_depth = texture(depth_map, texcoord).r;
	}
	return texcoord;
}

void main() {
	vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_position);
	vec2 texcoord = parallax_mapping1(displacement_map1, fs_in.texcoord, view_dir);
	if (texcoord.s < 0 || texcoord.s > 1 || texcoord.t < 0 || texcoord.t > 1)
		discard;

	vec3 normal = normalize(texture(normal_map1, texcoord).rgb * 2 - 1);
	vec3 light_dir = normalize(fs_in.tangent_light_dir);
	vec3 diffuse_color = texture(diffuse_map1, texcoord).rgb;

	// ambient
	vec3 ambient = diffuse_color * 0.2;

	// diffuse
	float diff = max(dot(light_dir, normal), 0.0);
	vec3 diffuse = diff * 0.8 * diffuse_color;

	// specular
	vec3 half_vec = normalize(light_dir + view_dir);
	float spec = pow(max(dot(half_vec, normal), 0.0), 64.0);
	vec3 specular = spec * 0.3 * diffuse_color;

	frag_color = vec4(ambient + diffuse + specular, 1.0);
}