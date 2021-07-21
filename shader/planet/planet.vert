#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
//							 normal;
//							 tangent;
//							 bitangent;
layout(location = 5) in mat4 instance_matrix; 

out VS_OUT {
	vec2 texcoord;
} vs_out;

uniform mat4 movement;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform bool is_planet;

void main() {
	if (!is_planet)
		gl_Position = projection * view * (movement * instance_matrix) * vec4(position, 1.0);
	else
		gl_Position = projection * view * model * vec4(position, 1.0);

	vs_out.texcoord = texcoord;
}