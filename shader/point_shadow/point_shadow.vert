#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position = projection * view * model * vec4(position, 1.0);
	vec3 model_position = vec3(model * vec4(position, 1.0));
	mat3 normal_matrix  = transpose(inverse(mat3(model)));
	vs_out.position = model_position;
	vs_out.texcoord = texcoord;
	vs_out.normal   = normal_matrix * normal;
}