#version 330 core
out vec4 frag_color;

in VS_OUT {
	vec2 texcoord;
} fs_in;

struct Light {
	vec3 position;
	vec3 color;
};

uniform sampler2D position_buffer;
uniform sampler2D normal_buffer;
uniform sampler2D albedo_buffer;
uniform vec3  view_pos;
uniform Light lights[32];
uniform float light_kc;
uniform float light_kl;
uniform float light_kq;

float get_light_attenuation(float d) {
	return 1.0 / (light_kc + light_kl * d + light_kq * d * d);
}

void main() {
	vec3 position = texture(position_buffer, fs_in.texcoord).rgb;
	vec3 albedo   = texture(albedo_buffer, fs_in.texcoord).rgb;
	vec3 N        = texture(normal_buffer, fs_in.texcoord).rgb;
	vec3 V		  = normalize(view_pos - position);

	vec3 ambient = 0.1 * albedo;
	vec3 Lo      = ambient;
	for (int i = 0; i < 32; ++i) {
		vec3 L = normalize(lights[i].position - position);
		vec3 H = normalize(L + V);
		float attenuation = get_light_attenuation(distance(lights[i].position, position));
		vec3 diffuse  = max(dot(L, N), 0.0) * albedo * lights[i].color;
		vec3 specular = pow(max(dot(H, N), 0.0), 64.f) * albedo * lights[i].color;
		Lo += (diffuse + specular) * attenuation;
	}

	vec3 color = Lo / (Lo + vec3(1.0));
	color	   = pow(color, vec3(1.0 / 2.2));
	frag_color = vec4(color, 1.0);
}