#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
	vec4	light_space_position;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_space_matrix;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0);
	
	mat3 normal_matrix = transpose(inverse(mat3(model)));
	vec4 model_position = model * vec4(position, 1.0);

	vs_out.position = vec3(model_position);
	vs_out.texcoord = texcoord;
	vs_out.normal = normal_matrix * normal;
	vs_out.light_space_position = light_space_matrix * model_position;
}