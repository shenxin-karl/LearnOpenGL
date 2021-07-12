#version 330 core
#include "brdf.frag"
#include "importance_sample.frag"

out vec4 frag_color;
in vec3 local_position;
uniform float roughness;
uniform samplerCube env_map;

vec3 prefilter(vec3 local_pos) {
	vec3 N = normalize(local_pos);
	vec3 R = N;
	vec3 V = N;

	float total_weight   = 0.0;
	vec3 prefilted_color = vec3(0.0);
	const uint SAMPLE_COUNT = 1024u;
	for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
		vec2 Xi = hammersley(i, SAMPLE_COUNT);
		vec3 H  = importance_sample_ggx(Xi, N, roughness);
		vec3 L  = normalize(2.0 * dot(V, H) * H - V);
		float NdotL = max(dot(N, L), 0.0);
		if (NdotL > 0.0) {
			prefilted_color += texture(env_map, L).rgb;
			total_weight	+= NdotL;
		}
	}
	prefilted_color /= total_weight;
	return prefilted_color;
}

void main() {
	vec3 color = prefilter(local_position);
	frag_color = vec4(color, 1.0);
}