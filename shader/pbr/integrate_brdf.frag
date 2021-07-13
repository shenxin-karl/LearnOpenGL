#version 330 core
#include "brdf.frag"
#include "importance_sample.frag"

in vec2 our_texcoord;
out vec2 frag_color;
uniform samplerCube env_map;

vec2 integrate_brdf(float NdotL, float roughness) {
	vec3 V;
	V.x = sqrt(1.0 - NdotL * NdotL);
	V.y = 0.0;
	V.z = NdotL;

	float k = roughness * roughness / 2.0;
	const uint SAMPLE_COUNT = 1024u;
	vec3  N = vec3(0, 0, 1);
	float A = 0.0;
	float B = 0.0;

	for (uint i = 0u; i < SAMPLE_COUNT; ++i) {
		vec2 Xi = hammersley(i, SAMPLE_COUNT);
		vec3 H  = importance_sample_ggx(Xi, N, roughness);
		vec3 L  = normalize(2.0 * dot(V, H) * H - V);
		
		float NdotH = max(H.z, 0.0);
		float NdotL = max(L.z, 0.0);
		float HdotV = max(dot(V, H), 0.0);
		if (NdotL > 0.0) {
			float G     = gemontry_smith(N, V, L, k);
			float G_vis = (G * HdotV) / (NdotH * HdotV);
			float Fc    = pow(1.0 - HdotV, 5.0);
			A += G_vis * (1 - Fc);
			B += G_vis * Fc;
		}
	}
	A /= SAMPLE_COUNT;
	B /= SAMPLE_COUNT;
	return vec2(A, B);
}

void main() {
	frag_color = integrate_brdf(our_texcoord.s, our_texcoord.t);
}