#version 330 core

in VS_OUT {
	vec3	position;
	vec2	texcoord;
	vec3	tangent_light_dir;
	vec3	tangent_view_pos;
	vec3	tangent_position;
} fs_in;

uniform sampler2D diffuse_map1;
uniform sampler2D normal_map1;
uniform sampler2D displacement_map1;
uniform float displacement_scale;

out vec4 frag_color;

vec2 parallax_mapping(sampler2D depth_map, vec2 texcoord, vec3 view_dir) {
	float depth = texture(depth_map, texcoord).r;
	vec2 p = view_dir.xy / view_dir.z * (depth * displacement_scale);
	return texcoord - p;
}


vec2 step_parallax_mapping(sampler2D depth_map, vec2 texcoord, vec3 view_dir) {
	float depth = texture(depth_map, texcoord).r;
	vec2 p = view_dir.xy / view_dir.z * (depth * displacement_scale);
	const float min_layers = 8;
	const float max_layers = 32;
	float layers = mix(max_layers, min_layers, abs(view_dir.z));
	float delta_depth = 1.f / layers;
	vec2 delta_p = p / layers;
	float curr_depth = 1.0;
	float expect_depth = 0.0;

	while (curr_depth > expect_depth) {		// find the first curr_depth less expect_depth
		expect_depth += delta_depth;
		texcoord -= delta_p;
		curr_depth = texture(depth_map, texcoord).r;
	}
	return texcoord;
}

vec2 parallax_occlusion_mapping(sampler2D depth_map, vec2 texcoord, vec3 view_dir) {
/*
	float depth = texture(depth_map, texcoord).r;
	vec2 p = view_dir.xy / view_dir.z * (depth * displacement_scale);
	const float min_layers = 8;
	const float max_layers = 32;
	float layers = mix(min_layers, max_layers, abs(view_dir.z));

	float delta_depth = 1 / layers;
	vec2 delta_p = p / layers;
	float curr_depth = 1.0;
	float expect_depth = 0.0;
	while (curr_depth > expect_depth) {
		expect_depth += delta_depth;
		texcoord -= delta_p;
		curr_depth = texture(depth_map, texcoord).r;
	}

	vec2 prev_texcoord = texcoord + delta_p;
	float prev_depth = texture(depth_map, prev_texcoord).r;
	float after_depth = expect_depth - curr_depth;
	float before_depth = prev_depth - (expect_depth - delta_depth);
	float weight = after_depth / (after_depth + before_depth);
	return mix(texcoord, prev_texcoord, weight);
*/
    // number of depth layers
    const float minLayers = 10;
    const float maxLayers = 20;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), view_dir)));  
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = view_dir.xy / view_dir.z * displacement_scale; 
    vec2 deltaTexCoords = P / numLayers;
  
    // get initial values
    vec2  currentTexCoords     = texcoord;
    float currentDepthMapValue = texture(depth_map, currentTexCoords).r;
      
    while(currentLayerDepth < currentDepthMapValue)
    {
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depth_map, currentTexCoords).r;  
        // get depth of next layer
        currentLayerDepth += layerDepth;  
    }
    
    // -- parallax occlusion mapping interpolation from here on
    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depth_map, prevTexCoords).r - currentLayerDepth + layerDepth;
 
    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

void main() {
	vec3 view_dir = normalize(fs_in.tangent_view_pos - fs_in.tangent_position);
	vec2 texcoord = step_parallax_mapping(displacement_map1, fs_in.texcoord, view_dir);
	if (texcoord.s < 0.0 || texcoord.s > 1.0 || texcoord.t < 0.0 || texcoord.t > 1.0)
		discard;

	vec3 normal = normalize(texture(normal_map1, texcoord).rgb * 2 - 1);
	vec3 light_dir = normalize(fs_in.tangent_light_dir);
	vec3 diffuse_color = texture(diffuse_map1, texcoord).rgb;

	// ambient
	vec3 ambient = diffuse_color * 0.1;

	// diffuse
	float diff = max(dot(light_dir, normal), 0.0);
	vec3 diffuse = diff * 0.5 * diffuse_color;

	// specular
	vec3 half_vec = normalize(light_dir + view_dir);
	float spec = pow(max(dot(half_vec, normal), 0.0), 64.0);
	vec3 specular = spec * 0.1 * diffuse_color;

	vec3 color = ambient + diffuse + specular;
	color = pow(color, vec3(1 / 2.2));
	frag_color = vec4(color, 1.0);
}