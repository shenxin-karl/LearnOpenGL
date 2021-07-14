#version 330 core
#include "../pbr/brdf.frag"

out vec4 frag_color;
in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
	vec3	tangent;
	vec3	bitangent;
} fs_in;

struct Light {
	vec3	position;
	vec3	color;
};

uniform samplerCube irradiance_map;
uniform samplerCube prefilter_map;
uniform sampler2D   brdf_lut_map;
uniform sampler2D	albedo_map;
uniform sampler2D	normal_map;
uniform sampler2D	metallic_map;
uniform sampler2D	roughness_map;
uniform sampler2D	ambient_occlusion_map;
uniform vec3		view_pos;
uniform Light	    lights[4];

vec3 get_normal() {
	vec3 T = normalize(fs_in.tangent);
	vec3 B = normalize(fs_in.bitangent);
	vec3 N = normalize(fs_in.normal);
	mat3 TBN = mat3(T, B, N);
	vec3 normal = texture(normal_map, fs_in.texcoord).rgb * 0.5 + 0.5;
	return normalize(TBN * normal);
}

void main() {
	vec3 albedo     = texture(albedo_map, fs_in.texcoord).rgb;
	float ao        = texture(ambient_occlusion_map, fs_in.texcoord).r;
	float metallic  = texture(metallic_map, fs_in.texcoord).r;
	float roughness = texture(roughness_map, fs_in.texcoord).r;
	float k			= roughness * roughness / 8.0;

	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 N  = get_normal();
	vec3 V  = normalize(view_pos - fs_in.position);
	vec3 R	= reflect(-V, N);

	vec3 Lo = vec3(0.0);
	for (int i = 0; i < 4; ++i)  {
		vec3 light_vec    = (lights[i].position - fs_in.position);
		float distances   = length(light_vec);
		float attenuation = 1.0 / (distances * distances);
		vec3  radiance    = lights[i].color * attenuation;

		vec3 L = normalize(light_vec);
		vec3 H = normalize(L + V);

		vec3 F    = fresnel_schlick(max(dot(H, V), 0.0), F0);
		float G   = gemontry_smith(N, V, L, k);
		float NDF = distribution_ggx(N, H, roughness);

		vec3 nom      = NDF * G * F;
		float denom   = 4 * max(dot(L, N), 0.0) * max(dot(V, N), 0.0);
		vec3 specular = nom / max(denom, 0.001);

		vec3 Kd     = (1.0 - F) * (1.0 - metallic);
		float NdotL = max(dot(N, L), 0.0);
		Lo         += (Kd * albedo / PI + specular) * radiance * NdotL;
	}

	float NdotV	    = max(dot(N, V), 0.0);
	vec3 F          = fresnel_schlick_roughness(NdotV, F0, roughness);
	vec3 Kd         = (1.0 - F) * (1.0 - metallic);
	vec3 irradiance = texture(irradiance_map, N).rgb;
	vec3 diffuse    = Kd * irradiance * albedo;

	vec3 prefilter_color = textureLod(prefilter_map, R, roughness).rgb;
	vec2 env_brdf	     = texture(brdf_lut_map, vec2(NdotV, roughness)).rg;
	vec3 specular		 = prefilter_color * (F * env_brdf.x + env_brdf.y);

	vec3 ambient = (diffuse + specular) * ao;
	vec3 color   = Lo + ambient;
	color        = color / (vec3(1.0) + color);
	color		 = pow(color, vec3(1.0 / 2.2));
	frag_color	 = vec4(color, 1.0);
}