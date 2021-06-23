#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

out VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
	vec3	color;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_dir;
uniform vec3 eye_pos;


void main() {
	gl_Position = projection * view * model * vec4(position, 1.f);
	vs_out.position = vec3(model * vec4(position, 1.f));
	vs_out.texcoord = texcoord;
	vs_out.normal = transpose(inverse(mat3(model))) * normal;
	vs_out.color = normalize(vec3(texcoord, 1.f));
}