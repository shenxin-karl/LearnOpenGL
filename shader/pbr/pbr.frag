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
uniform vec3  view_pos;
uniform vec3  albedo;
uniform float metallic;
uniform float roughness;
uniform samplerCube irradiance_env_map;		// 漫反射辐照度环境贴图
uniform samplerCube prefilter_map;		
uniform sampler2D	brdf_lut_map;			

const float PI = 3.1415926535898;

vec3 fresnel_schlick(vec3 H, vec3 V, vec3 F0) {
	float HdotV = max(dot(H, V), 1.0);
	return F0 + (1.0 - F0) * pow(1.0 - HdotV, 5.0);
}

vec3 fresnel_schlick_roughness(vec3 N, vec3 V, vec3 F0, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1 - NdotV, 5.0);
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
	float a      = roughness * roughness;
	float a2     = a * a;
	float NdotH  = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	float nom    = a2;
	float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
	denom	     = PI * denom * denom;
	return nom / denom;
}

float geomtry_schlick_ggx(float NdotV, float roughness) {
	float r     = roughness + 1.0;
	float k     = (r * r) / 8.0;
	float nom   = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	return nom / denom;
}

float geometry_smith(vec3 N, vec3 L, vec3 V, float roughness) {
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1  = geomtry_schlick_ggx(NdotV, roughness);
	float ggx2  = geomtry_schlick_ggx(NdotL, roughness);
	return ggx1 * ggx2;
}

void main() {
	vec3 V = normalize(view_pos - fs_in.position);
	vec3 N = normalize(fs_in.normal);
	vec3 R = reflect(-V, N);

	vec3 F0 = vec3(0.04);
	F0      = mix(F0, albedo, metallic);

	vec3 Lo = vec3(0);
	for (int i = 0; i < 4; ++i) {
		float distances   = distance(lights[i].position, fs_in.position);
		float attenuation = 1.0 / (distances * distances);
		vec3  radiances   = lights[i].color * attenuation;

		vec3 L = normalize(lights[i].position - fs_in.position);
		vec3 H = normalize(V + L);

		vec3 F    = fresnel_schlick(H, V, F0);
		float NDF = distribution_ggx(N, H, roughness);
		float G   = geometry_smith(N, L, V, roughness);

		vec3 nom      = NDF * G * F;
		float denom   = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);	
		vec3 specular = nom / max(denom, 0.001);		// 防止分母为 0 

		vec3 Ks = F;
		vec3 Kd = vec3(1.0) - Ks;
		Kd *= 1.0 - metallic;	

		float NdotL = max(dot(N, L), 0.0);
		Lo += (Kd * albedo / PI + specular) * radiances * NdotL;
	}

	// 使用 IBM 预处理的漫反射
	vec3 Ks		    = fresnel_schlick_roughness(N, V, F0, roughness);
	vec3 Kd			= (1.0 - Ks) * (1.0 - metallic);
	vec3 irradiance = texture(irradiance_env_map, N).rgb;
	vec3 diffuse    = Kd * irradiance * albedo;
	
	const float MAX_ROUGHNESS_LEVEL = 4.0;
	vec3 prefilter_color = textureLod(prefilter_map, R, roughness * MAX_ROUGHNESS_LEVEL).rgb;
	vec3 env_brdf        = texture(brdf_lut_map, vec2(max(dot(N, V), 0.0), roughness)).rgb;
	vec3 specular		 = prefilter_color * (Ks * env_brdf.x + env_brdf.y);
	vec3 ambient		 = diffuse + specular;

	vec3 color   = Lo + ambient;
	color		 = color / (color + vec3(1));		// HDR
	color		 = pow(color, vec3(1 / 2.2));		// gamma
	frag_color	 = vec4(color, 1.0);
}