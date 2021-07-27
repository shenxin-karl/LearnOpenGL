#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out VS_OUT {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	vec4 view_position = view * model * vec4(position, 1.0);
	gl_Position = projection * view_position;
	mat3 normal_matrix = transpose(inverse(mat3(view * model)));
	vs_out.position  = view_position.xyz;
	vs_out.texcoord  = texcoord;
	vs_out.normal    = normal_matrix * normal;
	vs_out.tangent   = normal_matrix * tangent;
	vs_out.bitangent = normal_matrix * bitangent;
}