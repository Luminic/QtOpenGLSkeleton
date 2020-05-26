#version 450

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 bloom_color;

in vec2 texture_coordinate;

uniform sampler2D screen_texture;
uniform bool greyscale;

uniform float bloom_threshold_upper;
uniform float bloom_threshold_lower;
uniform int bloom_interpolation;

// // Muliplying screen space coordinates (w/ depth) by inverse_perspective_view_transform will get the global space coordnates
// // Depth is located in the alpha value of the colorbuffers (it was the easiest way to get that data)
// uniform mat4 inverse_perspective_view_transform;
//
// struct DirLight {
// 	vec3 direction;
//
// 	vec3 color;
// 	float ambient;
// 	float diffuse;
// 	float specular;
//
// 	sampler2D shadow_map;
// 	mat4 light_space;
// };
//
// struct Light {
// 	vec3 position;
//
// 	vec3 color;
// 	float ambient;
// 	float diffuse;
// 	float specular;
//
// 	float constant;
// 	float linear;
// 	float quadratic;
//
// 	int samples;
// 	float sample_radius;
//
// 	samplerCube shadow_cubemap;
// };
//
// uniform int nr_dirlights;
// uniform DirLight dirlights[1];
//
// uniform int nr_lights;
// uniform Light lights[1];
//
// // Used for stepping from the global coordinate to the camera position
// uniform vec3 camera_position;
//
// float in_dirlight_shadow(DirLight dirlight, bool use_pcf) {
// 	vec4 position_light_space = dirlight.light_space * vec4(fs_in.fragment_position, 1.0f);
// 	vec3 projected_coordinates = position_light_space.xyz / position_light_space.w;
// 	projected_coordinates = projected_coordinates * 0.5f + 0.5f;
// 	if (projected_coordinates.z > 1.0f) return 0.0f;
//
// 	float current_depth = max(0.001f,projected_coordinates.z); // If a fragment has a depth of <=0.0f, then it will always be lit... even if its behind the light source
//
// 	if (use_pcf) {
// 		float shadow = 0.0f;
// 		vec2 texel_size = 1.0f / textureSize(dirlight.shadow_map, 0);
// 		for (int x=-1; x<=1; x++) {
// 			for (int y=-1; y<=1; y++) {
// 				float pcf_depth = texture(dirlight.shadow_map, projected_coordinates.xy+vec2(x,y)*texel_size).r;
// 				shadow += current_depth > pcf_depth+SHADOW_BIAS ? 1.0f : 0.0f;
// 			}
// 		}
// 		shadow /= 9.0f;
//
// 		return shadow;
// 	} else {
// 		float closest_depth = texture(dirlight.shadow_map, projected_coordinates.xy).r;
// 		return current_depth > closest_depth+SHADOW_BIAS ? 1.0f : 0.0f;
// 	}
// }
//
// vec3 dirlight_vis(DirLight dirlight) {
// 	vec4 position_light_space = dirlight.light_space * vec4(fs_in.fragment_position, 1.0f);
// 	vec3 projected_coordinates = position_light_space.xyz / position_light_space.w;
// 	projected_coordinates = projected_coordinates * 0.5f + 0.5f;
// 	// if (projected_coordinates.z > 1.0f) return vec3(1.0f,0.0f,0.0f);
// 	// float closest_depth = texture(dirlight.shadow_map, projected_coordinates.xy).r;
// 	return projected_coordinates;
// 	// float current_depth = max(0.001f,projected_coordinates.z);
// 	// float closest_depth = texture(dirlights[1].shadow_map, projected_coordinates.xy).r;
// 	// return closest_depth.xxx;
// 	// return current_depth > closest_depth+SHADOW_BIAS ? 1.0f.xxx : 0.0f.xxx;
// }
//
// vec3 sample_offset_directions[26] = vec3[](
// 	vec3( 0,  0,  0), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
// 	vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
// 	vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
// 	vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
// 	vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1),
// 	vec3( 0,  0,  1), vec3( 0,  0, -1), vec3( 0,  1,  0), vec3( 1,  0,  0),
// 	vec3( 0, -1,  0), vec3(-1,  0,  0)
// );
//
// float in_pointlight_shadow(Light pointlight, vec3 position, bool use_pcf) {
// 	vec3 position_to_light = position - pointlight.position;
// 	float current_depth = length(position_to_light);
// 	float shadow = 0.0f;
//
// 	for (int i=0; i<pointlight.samples; i++) {
// 		float closest_depth = texture(pointlight.shadow_cubemap, position_to_light+sample_offset_directions[i]*pointlight.sample_radius).r;
// 		closest_depth *= 45.0f;
// 		shadow += current_depth > closest_depth+SHADOW_BIAS ? 1.0f : 0.0f;
// 	}
// 	shadow /= pointlight.samples;
//
// 	return shadow;
// }

void main() {
  vec3 col;
  if (greyscale) {
    col = texture(screen_texture, texture_coordinate).rrr;
  } else {
    col = texture(screen_texture, texture_coordinate).rgb;
  }
  frag_color = vec4(col, 1.0f);

  float brightness = max(max(col.r, col.g), col.b);//dot(col, vec3(0.2126, 0.7152, 0.0722));
  if (brightness >= bloom_threshold_upper) {
    bloom_color = frag_color;
  } else if (brightness <= bloom_threshold_lower) {
    bloom_color = vec4(0.0f.xxx, 1.0f);
  } else {
    #define a bloom_threshold_lower
    #define b bloom_threshold_upper
    #define x brightness
    #define pi 3.1415926535f

    float interpolate;
    if (bloom_interpolation == 0) { // Linear interpolation
      interpolate = (x-a)/(b-a);
    }
    else { // Sinusoidal interpolation
      #define s pi/(b-a)
      #define o (b+a)/2
      interpolate = sin((x-o)*s)/2.0f+0.5f;
    }
    bloom_color = vec4(col*interpolate, 1.0f);
  }
}
