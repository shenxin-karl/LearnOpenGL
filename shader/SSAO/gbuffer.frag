#version 330 core
layout(location = 0) out vec4 g_position;
layout(location = 1) out vec3 g_normal;
layout(location = 2) out vec3 g_albedo_diff;
layout(location = 3) out vec3 g_albedo_spec;

in VS_OUT {
	vec3 position;
	vec2 texcoord;
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
} fs_in;

uniform sampler2D normal_map;
uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform float NEAR;
uniform float FAR;
uniform bool is_plane;

vec3 get_normal() {
	vec3 T = normalize(fs_in.tangent);
	vec3 B = normalize(fs_in.bitangent);
	vec3 N = normalize(fs_in.normal);
	mat3 TBN = mat3(T, B, N);
	vec3 normal = texture(normal_map, fs_in.texcoord).rgb * 0.5 + 0.5;
	return normalize(TBN * normal);
}

float linear_depth() {
    float z = gl_FragCoord.z * 2.0 - 1.0; // Back to NDC 
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));	
}

void main() {
	g_position = vec4(fs_in.position, linear_depth());
	if (!is_plane) {
		g_normal      = get_normal();
		g_albedo_diff = texture(diffuse_map, fs_in.texcoord).rgb;
		g_albedo_spec = texture(specular_map, fs_in.texcoord).rgb;
	} else {
		g_normal	  = normalize(fs_in.normal);
		g_albedo_diff = texture(diffuse_map, fs_in.texcoord).rgb;
		g_albedo_spec = g_albedo_diff;
	}
}