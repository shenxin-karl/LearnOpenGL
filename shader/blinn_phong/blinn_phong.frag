#version 330 core

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
	vec3	color;
} fs_in;

uniform vec3 light_dir;
uniform vec3 eye_pos;
uniform sampler2D diffuse_map1;

out vec4 frag_color;

void main() {
	vec3 diffuse_color = texture(diffuse_map1, fs_in.texcoord).rgb;

	// ambient 
	vec3 ambient = 0.3 * diffuse_color;

	// diffuse
	vec3 normal = normalize(fs_in.normal);
	float diff = max(dot(light_dir, normal), 0.f);
	vec3 diffuse = diff * 0.8 * diffuse_color;

	// specular
	vec3 view_dir = normalize(eye_pos - fs_in.position);
	vec3 half_vec = normalize(view_dir + light_dir);
	float spec = pow(max(dot(half_vec, normal), 0.f), 64.f);
	vec3 specular = spec * 0.3 * diffuse_color;

	frag_color = vec4(ambient + diffuse + specular, 1.f);
}