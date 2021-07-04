#version 330 core

out vec4 frag_color;

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
} fs_in;

struct Light {
	vec3	position;
	vec3	color;
};
uniform Light lights[4];

uniform vec3 look_from;
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
const float PI = 3.14159265359;

vec3 freshnel_scklick(float cos_theta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cos_theta, 5.0);
}

float geometry_schlick_ggx(float NdotV, float roughness) {
	float r = (roughness + 1);
	float k = (r * r) / 8;
	float nom   = NdotV;
	float denom = NdotV * (1 - k) + k;
	return nom / denom;
}

float geometry_smith(vec3 N, vec3 L, vec3 V, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1  = geometry_schlick_ggx(NdotV, roughness);
	float ggx2  = geometry_schlick_ggx(NdotL, roughness);
	return ggx1 * ggx2;
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
	float a      = roughness * roughness;
	float a2     = a * a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH ;
	float nom    = a2;
	float denom  = NdotH2 * (a2 - 1) + 1;
	denom       = PI * denom * denom;
	return nom / denom;
}

void main() {
	vec3 V = normalize(look_from - fs_in.position);
	vec3 N = normalize(fs_in.normal);

	vec3 F0 = vec3(0.04);
	F0      = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0);
	for (int i = 0; i < 1; ++i) {
		vec3 light_vec    = lights[i].position - fs_in.position;
		float distances   = length(light_vec);
		float attenuation = 1.0 / (distances * distances);
		vec3 radiance     = lights[i].color * attenuation;

		vec3 L = normalize(light_vec);
		vec3 H = normalize(V + L);
		float spec = max(dot(H, N), 0.0);

		vec3  F   = freshnel_scklick(max(dot(H, V), 0.0), F0);
		float G   = geometry_smith(N, L, V, roughness);
		float NDF = distribution_ggx(N, H, roughness);

		vec3 nom      = F;
		float denom   = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		vec3 specular = nom / denom;

		vec3 Ks = F;
		vec3 Kd = 1.0 - Ks;
		Kd *= (1.0 - metallic);

		float NdotL = max(dot(N, L), 0.0);
		Lo += (Kd * albedo / PI + specular) * NdotL * radiance;
	}

	vec3 ambient = vec3(0.03) * albedo;
	vec3 color   = ambient + Lo;
	color        = color / (color + vec3(1));
	color		 = pow(color, vec3(1 / 2.2));
	frag_color   = vec4(color, 1.0);
}