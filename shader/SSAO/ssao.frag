#version 330 core
out vec4 frag_color;

in VS_OUT {
	vec2 texcoord;
} fs_in;

uniform sampler2D position_buffer;
uniform sampler2D normal_buffer;
uniform sampler2D albedo_diff_buffer;
uniform sampler2D albedo_spec_buffer;
uniform sampler2D ssao_texture;
uniform vec3	  light_position;
uniform vec3	  light_color;
uniform int		  width;

void main() {
	if (gl_FragCoord.x < (width / 2)) {
		vec3 color = vec3(texture(ssao_texture, fs_in.texcoord).r);
		frag_color = vec4(color, 1.0);
		return;
	}

	vec3 position    = texture(position_buffer, fs_in.texcoord).xyz;
	vec3 albedo_diff = texture(albedo_diff_buffer, fs_in.texcoord).rgb;
	vec3 albedo_spec = texture(albedo_spec_buffer, fs_in.texcoord).rgb;
	vec3 N		     = texture(normal_buffer, fs_in.texcoord).xyz;
	vec3 V			 = normalize(vec3(0) - position);
	vec3 L			 = normalize(light_position - position);
	vec3 H		     = normalize(L + V);
	float ao		 = texture(ssao_texture, fs_in.texcoord).r;
	float distances   = distance(light_position, position);
	float attenuation = 1.0 / (distances * distances);

	vec3 ambient  = vec3(0.03) * ao * albedo_diff;
	vec3 diffuse  = max(dot(N, L), 0.0) * attenuation * albedo_diff * light_color;
	vec3 specular = pow(max(dot(N, H), 0.0), 64.0) * attenuation * albedo_spec * light_color;
	vec3 color    = ambient + diffuse + specular;
	color		  = color / (color + vec3(1.0));
	color         = pow(color, vec3(1.0 / 2.2));
	frag_color    = vec4(color, 1.0);
}