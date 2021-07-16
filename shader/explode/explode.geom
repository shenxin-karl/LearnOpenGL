#version 330 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform float offset;

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
} gs_in[];

out GEOM_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	normal;
} gemo_out;

vec3 get_normal() {
	vec3 v1 = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 v2 = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
	return normalize(cross(v1, v2));
}

void output_attr(vec3 position, int index) {
	gemo_out.position = position;
	gemo_out.texcoord = gs_in[index].texcoord;
	gemo_out.normal   = gs_in[index].normal;
}

void explobe() {
	vec3 normal = get_normal();
	for (int i = 0; i < 3; ++i) {
		vec4 position = gl_in[i].gl_Position + vec4(normal * offset, 0.0);
		gl_Position   = position;
		output_attr(position.xyz / position.w, i);
		EmitVertex();
	}
	EndPrimitive();
}

void main() {
	explobe();
}