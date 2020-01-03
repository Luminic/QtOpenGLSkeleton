#version 420

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 bloom_color;

in vec2 texture_coordinate;

uniform sampler2D screen_texture;
uniform sampler2D other_textures[1];

uniform int display_type;
uniform float exposure;
uniform float bloom_threshold_upper;
uniform float bloom_threshold_lower;
uniform int bloom_interpolation;

void main() {
  vec3 col;
  switch (display_type) {
    case 0:{
      vec3 scr_col = texture(screen_texture, texture_coordinate).rgb;
      vec4 vol_col = texture(other_textures[0], texture_coordinate);
      col = mix(scr_col, vol_col.rgb, 1.0f-vol_col.a);
      // Exposure mapping
      col = vec3(1.0f) - exp(-col * exposure);
      break;}
    case 1:
      col = texture(screen_texture, texture_coordinate).rrr;
      break;
    case 2:
      col = texture(screen_texture, texture_coordinate).rgb;
      break;
    default:
      col = texture(screen_texture, texture_coordinate).rgb;
      // Exposure mapping
      col = vec3(1.0f) - exp(-col * exposure);
      break;
  }
  frag_color = vec4(col, 1.0f);

  float brightness = max(max(col.r, col.g), col.b);//dot(col, vec3(0.2126, 0.7152, 0.0722));
  if (brightness >= bloom_threshold_upper) {
    bloom_color = frag_color;
  } else if (brightness <= bloom_threshold_lower) {
    bloom_color = vec4(0.0f.xxx, 1.0f);
  } else {
    float a = bloom_threshold_lower;
    float b = bloom_threshold_upper;
    float x = brightness;
    float pi = 3.1415926535f;
    float interpolate;
    // Linear interpolation
    if (bloom_interpolation == 0) {
      interpolate = (x-a)/(b-a);
    }
    // Sinusoidal interpolation
    else {
      float s = pi/(b-a);
      float o = (b+a)/2;
      interpolate = sin((x-o)*s)/2.0f+0.5f;
    }
    bloom_color = vec4(col*interpolate, 1.0f);
  }
  // bloom_color = max(bloom_color, 0.0f.xxxx);
  // bloom_color = min(bloom_color, 5.0f.xxxx);
  // bloom_color.a = 1.0f;
  //frag_color = vec4(vec3(display_type), 1.0f);
  //frag_color = vec4(vec3(texture(screen_texture, texture_coordinate).r), 1.0f);
}
