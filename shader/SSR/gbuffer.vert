#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out VS_OUT {
	vec4	position;
	vec2	texcoord;
	vec3	normal;
} vs_out;

void main() {
	vec4 view_position = view * model * vec4(position, 1.0);
	gl_Position = projection * view_position;
	mat3 normal_matrix = transpose(inverse(mat3(view * model)));
	vs_out.position = view_position;
	vs_out.texcoord = texcoord;
	vs_out.normal   = normal_matrix * normal;
}