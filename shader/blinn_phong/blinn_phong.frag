#version 330 core

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
} fs_in;

struct Light {
	vec3	light_dir;
	vec3	ambient;
	vec3	diffuse;
	vec3	specular;
	float	shininess;
};

uniform Light light;
uniform vec3 eye_pos;
uniform sampler2D diffuse_map1;

out vec4 frag_color;

void main() {
	vec3 diffuse_color = texture(diffuse_map1, fs_in.texcoord).rgb;

	// ambient 
	vec3 ambient = light.ambient * diffuse_color;

	// diffuse
	vec3 normal = normalize(fs_in.normal);
	float diff = max(dot(light.light_dir, normal), 0.f);
	vec3 diffuse = diff * light.diffuse * diffuse_color;

	// specular
	vec3 view_dir = normalize(eye_pos - fs_in.position);
	vec3 half_vec = normalize(view_dir + light.light_dir);
	float spec = pow(max(dot(half_vec, normal), 0.f), light.shininess);
	vec3 specular = spec * light.specular * diffuse_color;

	frag_color = vec4(ambient + diffuse + specular, 1.f);
}