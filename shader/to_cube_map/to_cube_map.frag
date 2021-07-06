#version 330 core

in vec3 local_position;
out vec4 frag_color;
uniform sampler2D texture_map;


// atan ·µ»Ø [-PI, +PI]
// asin ·µ»Ø [-PI/2, +PI/2]
const float PI = 3.141592654;
const vec2 inv_atan = vec2(0.5/PI, 1/PI);
vec2 sample_sphere_uv(vec3 v) {
	vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
	uv *= inv_atan;
	uv += 0.5;
	return uv;
}

void main() {
	vec2 uv = sample_sphere_uv(normalize(local_position));
	vec3 color = texture(texture_map, uv).rgb;
	frag_color = vec4(color, 1.0);
}