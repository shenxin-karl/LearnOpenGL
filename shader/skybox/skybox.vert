#version 330 core
layout(location = 0) in vec3 position;

out vec3 our_texcoord;
uniform mat4 view;
uniform mat4 projection;

void main() {
	gl_Position  = (projection * mat4(mat3(view)) * vec4(position, 1.0)).xyww;
	our_texcoord = position;
}