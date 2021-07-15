#version 330 core
layout(points) in;
// layout(points, max_vertices = 1) out;
layout(triangle_strip, max_vertices = 5) out;

void build_house(vec4 position) {
	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(+0.2, -0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(-0.2, +0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(+0.2, +0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(+0.0, +0.4, 0.0, 0.0);
	EmitVertex();
	EndPrimitive();
}

void main() {
	// gl_Position = gl_in[0].gl_Position;
	// EmitVertex();
	// EndPrimitive();

	build_house(gl_in[0].gl_Position);
}