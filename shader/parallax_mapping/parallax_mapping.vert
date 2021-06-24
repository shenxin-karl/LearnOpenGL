#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	tangent_light_dir;
	vec3	tangent_view_pos;
	vec3	tangent_position;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 eye_pos;
uniform vec3 light_dir;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0);
	mat3 normal_matrix = transpose(inverse(mat3(model)));
	vec3 T = normalize(normal_matrix * tangent);
	vec3 B = normalize(normal_matrix * bitangent);
	vec3 N = normalize(normal_matrix * normal);
	mat3 TBN = transpose(mat3(T, B, N));

	vec3 model_position = vec3(model * vec4(position, 1.0));
	vs_out.position = model_position;
	vs_out.texcoord = texcoord;
	vs_out.tangent_light_dir = TBN * light_dir;
	vs_out.tangent_view_pos = TBN * eye_pos;
	vs_out.tangent_position = TBN * model_position;
}