#version 450

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 bloom_color;

in vec2 texture_coordinate;

uniform sampler2D screen_texture;
uniform bool greyscale;

uniform float bloom_threshold_upper;
uniform float bloom_threshold_lower;
uniform int bloom_interpolation;

// Muliplying screen space coordinates (w/ depth) by inverse_perspective_view_transform will get the global space coordnates
// Depth is located in the alpha value of the colorbuffers (it was the easiest way to get that data)
uniform mat4 inverse_perspective_transform;
uniform mat4 inverse_view_transform;

#mypreprocessor include "shader_components/light_structs.glsl"

uniform int nr_dirlights;
uniform DirLight dirlights[MAX_NR_DIRLIGHTS];

uniform int nr_lights;
uniform Light lights[MAX_NR_LIGHTS];

// Used for stepping from the global coordinate to the camera position
uniform vec3 camera_position;

#mypreprocessor include "shader_components/shadow_functions.glsl"
#mypreprocessor include "shader_components/misc_functions.glsl"

uniform int volumetric_samples;
uniform float volumetric_scattering;
uniform float volumetric_density;
uniform float scattering_direction;


vec3 calculate_world_space(vec3 screen_space, mat4 inverse_view, mat4 inverse_perspective) {
  vec4 clip_space_coordinates = vec4(screen_space.xy*2.0f-1.0f, non_linear_depth(screen_space.z)*2.0f-1.0f, 1.0f);
  vec4 view_space_coordinates = inverse_perspective * clip_space_coordinates;
  view_space_coordinates /= view_space_coordinates.w;
  return (inverse_view * view_space_coordinates).xyz;
}

void main() {
  vec4 col;
  if (greyscale) {
    col = texture(screen_texture, texture_coordinate).rrrr;
  } else {
    col = texture(screen_texture, texture_coordinate).rgba;
  }

  vec3 world_space_position = calculate_world_space(vec3(texture_coordinate, col.w), inverse_view_transform, inverse_perspective_transform);
  if (length(world_space_position) >= 100) {
    world_space_position = normalize(world_space_position)*100.0f;
  }

  int steps = volumetric_samples;
  vec3 camera_direction = normalize(camera_position - world_space_position);

  float albedo = volumetric_scattering;
  float tau = volumetric_density;
  vec3 L0 = 0.0f.xxx;
  vec3 s = camera_position-world_space_position;
  vec3 dl = s/steps;
  float step_size = length(s);
  float dl_size = length(dl);
  vec3 L = L0 * exp(-step_size*tau);
  float l = step_size-dl_size;
  vec3 x = world_space_position;
  for (int i=0; i<steps; i++) {
    x += dl;
    float v = in_dirlight_shadow(dirlights[0], x, false) == 0.0f ? 1.0f : 0.0f;
    float d = 1.0f;
    float Lin = exp(-d * tau) * v * dirlights[0].diffuse/4.0f/3.14159f/d/d;
    float Li = Lin * tau * albedo * henyey_greenstein(dot(normalize(dirlights[0].direction), -camera_direction), scattering_direction);
    L += Li * exp(-l*tau) * dl_size;
    l -= dl_size;
  }

  frag_color = vec4(L+col.rgb, 1.0f);

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
    bloom_color = vec4(col.xyz*interpolate, 1.0f);
  }
}
