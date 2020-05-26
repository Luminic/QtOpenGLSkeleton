#ifndef SHADOW_FUNCTIONS_GLSL
#define SHADOW_FUNCTIONS_GLSL

#define SHADOW_BIAS 0.001f

#mypreprocessor include "light_structs.glsl"

float in_dirlight_shadow(DirLight dirlight, vec3 fragment_position, bool use_pcf) {
	vec4 position_light_space = dirlight.light_space * vec4(fragment_position, 1.0f);
	vec3 projected_coordinates = position_light_space.xyz / position_light_space.w;
	projected_coordinates = projected_coordinates * 0.5f + 0.5f;
	if (projected_coordinates.z > 1.0f) return 0.0f;

	float current_depth = max(0.001f,projected_coordinates.z); // If a fragment has a depth of <=0.0f, then it will always be lit... even if its behind the light source

	if (use_pcf) {
		float shadow = 0.0f;
		vec2 texel_size = 1.0f / textureSize(dirlight.shadow_map, 0);
		for (int x=-1; x<=1; x++) {
			for (int y=-1; y<=1; y++) {
				float pcf_depth = texture(dirlight.shadow_map, projected_coordinates.xy+vec2(x,y)*texel_size).r;
				shadow += current_depth > pcf_depth+SHADOW_BIAS ? 1.0f : 0.0f;
			}
		}
		shadow /= 9.0f;

		return shadow;
	} else {
		float closest_depth = texture(dirlight.shadow_map, projected_coordinates.xy).r;
		return current_depth > closest_depth+SHADOW_BIAS ? 1.0f : 0.0f;
	}
}

vec3 dirlight_vis(DirLight dirlight, vec3 fragment_position) {
	vec4 position_light_space = dirlight.light_space * vec4(fragment_position, 1.0f);
	vec3 projected_coordinates = position_light_space.xyz / position_light_space.w;
	projected_coordinates = projected_coordinates * 0.5f + 0.5f;
	// if (projected_coordinates.z > 1.0f) return vec3(1.0f,0.0f,0.0f);
	// float closest_depth = texture(dirlight.shadow_map, projected_coordinates.xy).r;
	return projected_coordinates;
	// float current_depth = max(0.001f,projected_coordinates.z);
	// float closest_depth = texture(dirlights[1].shadow_map, projected_coordinates.xy).r;
	// return closest_depth.xxx;
	// return current_depth > closest_depth+SHADOW_BIAS ? 1.0f.xxx : 0.0f.xxx;
}

vec3 sample_offset_directions[26] = vec3[](
	vec3( 0,  0,  0), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
	vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1),
	vec3( 0,  0,  1), vec3( 0,  0, -1), vec3( 0,  1,  0), vec3( 1,  0,  0),
	vec3( 0, -1,  0), vec3(-1,  0,  0)
);

float in_pointlight_shadow(Light pointlight, vec3 fragment_position, bool use_pcf) {
	vec3 position_to_light = fragment_position - pointlight.position;
	float current_depth = length(position_to_light);
	float shadow = 0.0f;

	for (int i=0; i<pointlight.samples; i++) {
		float closest_depth = texture(pointlight.shadow_cubemap, position_to_light+sample_offset_directions[i]*pointlight.sample_radius).r;
		closest_depth *= 45.0f;
		shadow += current_depth > closest_depth+SHADOW_BIAS ? 1.0f : 0.0f;
	}
	shadow /= pointlight.samples;

	return shadow;
}

#endif
