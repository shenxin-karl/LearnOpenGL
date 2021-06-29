#version 330 core

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	tangent_light_dir;
	vec3	tangent_view_dir;
	vec3	tangent_position;
} fs_in;

uniform sampler2D diffuse_map1;
uniform sampler2D normal_map1;

out vec4 frag_color;

void main() {
	vec3 diffuse_color = texture(diffuse_map1, fs_in.texcoord).rgb;
	vec3 view_dir = normalize(fs_in.tangent_view_dir);
	vec3 light_dir = normalize(fs_in.tangent_light_dir);
	vec3 normal = normalize(texture(normal_map1, fs_in.texcoord).rgb * 2 - 1);
	
	// ambient
	vec3 ambient = 0.2 * diffuse_color;

	// diffuse
	float diff = max(dot(normal, light_dir), 0);
	vec3 diffuse = diff * 0.5 * diffuse_color;

	// specular
	vec3 half_vec = normalize(view_dir + light_dir);
	float spec = pow(max(dot(half_vec, normal), 0.0), 64);
	vec3 specular = spec * 0.3 * diffuse_color;

	frag_color = vec4(ambient + diffuse + specular, 1.0);
}