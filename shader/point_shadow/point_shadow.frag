#version 330 core

out vec4 frag_color;

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
} fs_in;

uniform vec3 view_pos;
uniform vec3 light_pos;
uniform vec3 light_color;
uniform sampler2D diffuse_map;
uniform samplerCube depth_cube_map;
uniform float far_plane;
uniform bool is_shadow;

float calcuation_shadow(vec3 farg_pos, float NdotL) {
	vec3 frag_to_light  = farg_pos - light_pos;
	float curr_depth    = length(frag_to_light);
	float closest_depth = texture(depth_cube_map, frag_to_light).r * far_plane;
	float bias          = mix(0.1, 0.005, NdotL);
	return ((curr_depth - bias) <= closest_depth) ? 1.0 : 0.0;
}

float calcuation_shadow1(vec3 frag_pos, float NdotL) {
	vec3 frag_to_light  = frag_pos - light_pos;
	float curr_depth    = length(frag_to_light);
	float bias          = mix(0.1, 0.005, NdotL);
	curr_depth		   -= bias;

	const float offset = 0.1;
	const float sample_count = 3.0;
	const float sample_delta = (offset * 2.0) / sample_count;
	float shadow = 0.0;
	for (float x = -offset; x < offset; x += sample_delta) {
		for (float y = -offset; y < offset; y += sample_delta) {
			for (float z = -offset; z < offset; z += sample_delta) {
				vec3 sample_dir = frag_to_light + vec3(x, y, z);
				float closest_depth = texture(depth_cube_map, sample_dir).r * far_plane;
				if (curr_depth < closest_depth) 
					shadow += 1.0;
			}
		}
	}
	return shadow / (sample_count * sample_count * sample_count);
}

vec3 sampleOffsetDirections[20] = vec3[](
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float calcuation_shadow2(vec3 frag_pos, float NdotL) {
	vec3 frag_to_light  = frag_pos - light_pos;
	float curr_depth    = length(frag_to_light);
	float bias          = mix(0.1, 0.05, NdotL);
	curr_depth		   -= bias;

	float shadow = 0.0;
	for (int i = 0; i < 20; ++i) {
		vec3 sample_dir = frag_to_light + (sampleOffsetDirections[i] * 0.1);
		float closest_depth = texture(depth_cube_map, sample_dir).r * far_plane;
		if (curr_depth < closest_depth) 
			shadow += 1.0;
	}
	return shadow / (20.0);
}

void main() {
	vec3 N = normalize(fs_in.normal);
	vec3 L = normalize(light_pos - fs_in.position);
	vec3 V = normalize(view_pos - fs_in.position);
	vec3 H = normalize(L + V);
	vec3 diffuse_color   = texture(diffuse_map, fs_in.texcoord).rgb;
	float light_distance = distance(light_pos, fs_in.position);
	float attenumation   = 1.0 / (light_distance * light_distance);
	float shadow         = is_shadow ? calcuation_shadow2(fs_in.position, max(dot(N, L), 0.0)) : 1.0;
	vec3 radiance        = light_color * attenumation;

	vec3 ambient  = 0.1 * diffuse_color;
	vec3 diffuse  = 0.6 * max(dot(N, L), 0.0) * radiance * diffuse_color;
	vec3 specular = 0.3 * pow(max(dot(N, H), 0.0), 64.0) * radiance * diffuse_color;

	vec3 color = ambient + ((diffuse + specular) * shadow);
	color      = (color / (color + vec3(1)));
	frag_color = vec4(pow(color, vec3(1.0 / 2.2)), 1.0);
}