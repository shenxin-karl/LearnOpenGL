#version 330 core

out float frag_color;

in VS_OUT {
	vec2 texcoord;
} fs_in;

uniform sampler2D position_buffer;
uniform sampler2D normal_buffer;
uniform sampler2D noise_texture;
uniform vec3  samples[64];
uniform int   kernel_size;
uniform float radius;
uniform mat4  projection;

void main() {
	vec2 texsize	  = textureSize(position_buffer, 0);
	vec2 noise_scale  = vec2(texsize.x / 4.0, texsize.y / 4.0);
	vec3 position	  = texture(position_buffer, fs_in.texcoord).xyz;
	vec3 normal		  = texture(normal_buffer, fs_in.texcoord).xyz;
	vec3 random_vec	  = texture(noise_texture, fs_in.texcoord * noise_scale).xyz;
	vec3 tangent      = normalize(random_vec - normal * dot(random_vec, normal));
	vec3 bitangent    = cross(normal, tangent);
	mat3 TBN		  = mat3(tangent, bitangent, normal);

	float occlusion  = 0.0;
	for (int i = 0; i < kernel_size; ++i) {
		vec3 sample_vec = TBN * samples[i];
		sample_vec = position + sample_vec * radius;
		vec4 offset = vec4(sample_vec, 1.0);
		offset = projection * offset;
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;
		float sample_depth = -texture(position_buffer, offset.xy).w;
		float range_check = smoothstep(0.0, 1.0, radius / abs(position.z - sample_depth));
		occlusion += ((sample_vec.z < sample_depth) ? 1.0 : 0.0) * range_check;		// 采样点被遮挡
	}
	frag_color = 1.0 - (occlusion / kernel_size);
}