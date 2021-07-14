#ifndef BRDF
#define BRDF

#define PI 3.141592654

vec3 fresnel_schlick(float cos_theta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

vec3 fresnel_schlick_roughness(float cos_theta, vec3 F0, float roughness) {
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cos_theta, 5.0);
}

float gemontry_schlick_ggx(float NdotV, float k) {
	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	return nom / denom;
}

float gemontry_smith(vec3 N, vec3 V, vec3 L, float k) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1  = gemontry_schlick_ggx(NdotV, k);
	float ggx2  = gemontry_schlick_ggx(NdotL, k);
	return ggx1 * ggx2;
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
	float a      = roughness * roughness;
	float a2     = a * a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	float nom    = a2;
	float denom  = NdotH2 * (a2 - 1.0) + 1.0;
	denom		 = PI * denom * denom;
	return nom / denom;
}

#endif