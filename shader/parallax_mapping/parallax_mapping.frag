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


vec2 step_parallax_mapping(sampler2D depth_map, vec2 texcoord, vec3 view_dir) {
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

vec2 parallax_occlusion_mapping(sampler2D depth_map, vec2 texcoord, vec3 view_dir) {
	float depth = texture(depth_map, texcoord).r;
	vec2 p = view_dir.xy / view_dir.z * (depth * displacement_scale);
	const float min_layers = 8;
	const float max_layers = 32;
	float layers = mix(min_layers, max_layers, abs(view_dir.z));

	float delta_depth = 1 / layers;
	vec2 delta_p = p / layers;
	float curr_depth = 1.0;
	float expect_depth = 0.0;
	while (curr_depth > expect_depth) {
		expect_depth += delta_depth;
		texcoord -= delta_p;
		curr_depth = texture(depth_map, texcoord).r;
	}

	vec2 prev_texcoord = texcoord + delta_p;
	float prev_depth = texture(depth_map, prev_texcoord).r;
	float after_depth = expect_depth - curr_depth;
	float before_depth = prev_depth - (expect_depth - delta_depth);
	float weight = after_depth / (after_depth + before_depth);
	return mix(texcoord, prev_texcoord, weight);
}

void main() {
	vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_position);
	vec2 texcoord = parallax_occlusion_mapping(displacement_map1, fs_in.texcoord, view_dir);
	if (texcoord.s < 0 || texcoord.s > 1 || texcoord.t < 0 || texcoord.t > 1)
		discard;

	vec3 normal = normalize(texture(normal_map1, texcoord).rgb * 2 - 1);
	vec3 light_dir = normalize(fs_in.tangent_light_dir);
	vec3 diffuse_color = texture(diffuse_map1, texcoord).rgb;

	// ambient
	vec3 ambient = diffuse_color * 0.1;

	// diffuse
	float diff = max(dot(light_dir, normal), 0.0);
	vec3 diffuse = diff * 0.5 * diffuse_color;

	// specular
	vec3 half_vec = normalize(light_dir + view_dir);
	float spec = pow(max(dot(half_vec, normal), 0.0), 64.0);
	vec3 specular = spec * 0.1 * diffuse_color;

	vec3 color = ambient + diffuse + specular;
	color = pow(color, vec3(1 / 2.2));
	frag_color = vec4(color, 1.0);
}