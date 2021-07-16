#version 330 core
layout(points) in;
// layout(points, max_vertices = 1) out;
layout(triangle_strip, max_vertices = 5) out;

in VS_OUT {
	vec3	color;
} gs_in[];

out vec3 color;

void build_house(vec4 position) {
	color = gs_in[0].color;
	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(+0.2, -0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(-0.2, +0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(+0.2, +0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(+0.0, +0.4, 0.0, 0.0);
	color = vec3(1.0, 1.0, 1.0);
	EmitVertex();
	EndPrimitive();
}

void main() {
	// gl_Position = gl_in[0].gl_Position;
	// EmitVertex();
	// EndPrimitive();

	build_house(gl_in[0].gl_Position);
}