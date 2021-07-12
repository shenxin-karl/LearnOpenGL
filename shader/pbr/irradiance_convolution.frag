#version 330 core
#include "brdf.frag"

in vec3 local_position;
out vec4 frag_color;
uniform samplerCube env_map;

vec3 irradiance_sample(vec3 pos) {
	vec3 N     = normalize(pos);
	vec3 up    = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up         = normalize(cross(N, right));

	vec3 irradiance    = vec3(0);
	int sample_count   = 0;
	float sample_delta = 0.025;
	for (float phi = 0.0; phi < (2*PI); phi += sample_delta) {
		for (float theta = 0.0; theta < PI; theta += sample_delta) {
			float cos_phi = cos(phi), cos_theta = cos(theta);
			float sin_phi = sin(phi), sin_theta = sin(theta);
			vec3 tangent_wi = vec3(
				sin_theta * cos_phi,
				sin_theta * sin_phi,
				cos_theta
			);

			vec3 sample_wi = (tangent_wi.x * right) + (tangent_wi.y * up) + (tangent_wi.z * N);
			irradiance   += texture(env_map, sample_wi).rgb;
			sample_count += 1;
		}
	}
	return irradiance * (PI / sample_count);
}

void main() {
	vec3 color = irradiance_sample(local_position);
	frag_color = vec4(color, 1.0);
}
