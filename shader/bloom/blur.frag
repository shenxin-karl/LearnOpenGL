#version 330 core

in vec2 our_texcoord;
out vec4 frag_color;
uniform sampler2D color_map;
uniform bool horizontal;

const float weight[5] = float[](
	0.227027, 
    0.1945946, 
    0.1216216, 
    0.054054, 
    0.016216
);

void main() {
	vec2 tex_offset = 1.0 / textureSize(color_map, 0);
    vec3 result = vec3(0);
    if (horizontal) {
        for (int i = 0; i < 5; ++i) {
            vec2 offset = vec2(tex_offset.x, 0) * i;
            result += texture(color_map, our_texcoord + offset).rgb * weight[i];
            result -= texture(color_map, our_texcoord - offset).rgb * weight[i];
        }
    } else {
        for (int i = 0; i < 5; ++i) {
            vec2 offset = vec2(0, tex_offset.y) * i;
            result += texture(color_map, our_texcoord + offset).rgb * weight[i];
            result -= texture(color_map, our_texcoord - offset).rgb * weight[i];
        }
    }
    frag_color = vec4(result, 1.0);
}