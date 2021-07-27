#version 330 
out float frag_color;

in VS_OUT {
	vec2 texcoord;
} fs_in;

uniform sampler2D ssao_texture;
const int kernel_size = 2;

void main() {
	float result = 0.0;
	vec2 texsize = 1.0 / textureSize(ssao_texture, 0);
	for (int x = -kernel_size; x <= kernel_size; ++x) {
		for (int y = -kernel_size; y <= kernel_size; ++y) {
			vec2 offset = vec2(x, y) * texsize;
			vec2 texcoord = fs_in.texcoord + offset;
			result += texture(ssao_texture, texcoord).r;
		}
	}

	int count = kernel_size * 2 + 1;
	frag_color = result / (count * count);
}