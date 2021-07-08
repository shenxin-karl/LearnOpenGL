#version 330 core

in vec3 our_texcoord;
out vec4 frag_color;
uniform samplerCube env_map;
const float PI = 3.141592654;

void main() {
	vec3 N  = normalize(our_texcoord);
	vec3 up = vec3(0, 1, 0);
	vec3 right = normalize(cross(up, N));
	up = normalize(cross(N, right));

	vec3 irradiance = vec3(0);
	float sample_delta = 0.025;
	int sample_size = 0;
	for (float phi = 0.0; phi < (2*PI); phi += sample_delta) {
		for (float theta = 0.0; theta < (0.5*PI); theta += sample_delta) {
			float cos_theta = cos(theta), cos_phi = cos(phi);
			float sin_theta = sin(theta), sin_phi = sin(phi);
			vec3 tangent_wi = vec3(	
				sin_theta * cos_phi,
				sin_theta * sin_phi,
				cos_theta
			);

			vec3 sample_wi = (tangent_wi.x * right) + (tangent_wi.y * up) + (tangent_wi.z * N);
			vec3 color = texture(env_map, sample_wi).rgb;
			irradiance += color * cos_theta * sin_theta;
			++sample_size;
		}
	}

	irradiance = PI * (1.0 / sample_size) * irradiance;
	frag_color = vec4(irradiance, 1.0);
}	