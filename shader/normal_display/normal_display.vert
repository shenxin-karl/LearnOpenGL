#version 330 core
layout(location = 0) in vec3 position;
layout(location = 2) in vec3 normal;

out VS_OUT {
	vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position        = projection * view * model * vec4(position, 1.0);
	mat3 normal_matrix = transpose(inverse(mat3(model)));
	vs_out.normal      = normalize(vec3(projection * vec4(normal_matrix * normal, 0.0)));
}