#version 330 core

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
} fs_in;

uniform sampler2D diffuse_map1;
uniform vec3 eye_pos;
uniform vec3 light_pos[4];
uniform vec3 light_color[4];

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 bright_color;

void main() {
	vec3 normal = normalize(fs_in.normal);
	vec3 view_dir = normalize(eye_pos - fs_in.position);
	vec3 diffuse_color = texture(diffuse_map1, fs_in.texcoord).rgb;

	vec3 color = vec3(0);
	for (int i = 0; i < 4; ++i) {
		vec3 light_vec = light_pos[i] - fs_in.position;
		float light_distance = length(light_vec);
		float attenuation = 1.0 / (light_distance * light_distance);
		vec3 light_dir = normalize(light_vec);
		float cos_theta = max(dot(light_dir, normal), 0.0);
		color += light_color[i] * attenuation * cos_theta;
	}

	color *= diffuse_color;

#if 0
	// HDR
	color = color / (color + vec3(1));
	const float gamma = 2.2;
	frag_color = vec4(pow(color, vec3(1 / gamma)), 1.0);
#else
	frag_color = vec4(color, 1.0);
#endif
	
	float brightness = dot(frag_color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 0)
		bright_color = frag_color;
}