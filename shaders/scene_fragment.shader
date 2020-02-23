#version 450

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 bloom_color;

in vec2 texture_coordinate;

uniform sampler2D screen_texture;
uniform bool greyscale;

uniform float bloom_threshold_upper;
uniform float bloom_threshold_lower;
uniform int bloom_interpolation;

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
