#version 450

layout(location=0) out vec4 frag_color;

in vec2 texture_coordinate;

uniform sampler2D screen_texture;

const float EDGE_THRESHOLD_MIN = 0.0312f;
const float EDGE_THRESHOLD_MAX = 0.125f;
const int ITERATIONS = 8;
const float SUBPIXEL_QUALITY = 0.75f;

float rgb2luma(vec3 rgb) {
  // Assumes rgb is in linear space so the luma is transformed into gamma space
  return clamp(dot(rgb, vec3(0.299f,0.587f,0.114f)), 0.0f, 1.0f);
}

void main() {
  vec3 color_center = texture(screen_texture, texture_coordinate).rgb;
  vec2 texel_offset = 1.0f/textureSize(screen_texture, 0); // Get the size of a texel

  float luma_center = rgb2luma(color_center);
  float luma_down   = rgb2luma(texture(screen_texture, texture_coordinate+(texel_offset*vec2( 0,-1))).rgb);
  float luma_up     = rgb2luma(texture(screen_texture, texture_coordinate+(texel_offset*vec2( 0, 1))).rgb);
  float luma_left   = rgb2luma(texture(screen_texture, texture_coordinate+(texel_offset*vec2(-1, 0))).rgb);
  float luma_right  = rgb2luma(texture(screen_texture, texture_coordinate+(texel_offset*vec2( 1, 0))).rgb);

  float luma_min = min(luma_center, min(min(luma_down, luma_up), min(luma_left, luma_right)));
  float luma_max = max(luma_center, max(max(luma_down, luma_up), max(luma_left, luma_right)));

  float luma_range = luma_max - luma_min;

  // AA is not computed for
  if (luma_range < max(EDGE_THRESHOLD_MIN, luma_max*EDGE_THRESHOLD_MAX)) {
    frag_color = vec4(color_center, 1.0f);
    return;
  }

  float luma_down_left  = rgb2luma(texture(screen_texture, texture_coordinate+(texel_offset*vec2( 0,-1))).rgb);
  float luma_down_right = rgb2luma(texture(screen_texture, texture_coordinate+(texel_offset*vec2( 0,-1))).rgb);
  float luma_up_left    = rgb2luma(texture(screen_texture, texture_coordinate+(texel_offset*vec2( 0,-1))).rgb);
  float luma_up_right   = rgb2luma(texture(screen_texture, texture_coordinate+(texel_offset*vec2( 0,-1))).rgb);

  float luma_down_up = luma_down + luma_up;
  float luma_left_right = luma_left + luma_right;

  float luma_down_corners  = luma_down_left + luma_down_right;
  float luma_up_corners    = luma_up_left   + luma_up_right;
  float luma_left_corners  = luma_up_left   + luma_down_left;
  float luma_right_corners = luma_up_right  + luma_down_right;

  // Estimate the gradients along the horizontal and vertical axis
  float edge_horizontal = abs(luma_left_corners -2.0*luma_left) + abs(luma_down_up    -2.0*luma_center) + abs(luma_right_corners -2.0*luma_right);
  float edge_vertical   = abs(luma_up_corners   -2.0*luma_up  ) + abs(luma_left_right -2.0*luma_center) + abs(luma_down_corners  -2.0*luma_down );
  bool is_horizontal = edge_horizontal >= edge_vertical;

  // Get the luma of the texels perpendicular to the edge ...
  float luma1 = is_horizontal ? luma_down : luma_left ;
  float luma2 = is_horizontal ? luma_up   : luma_right;
  // ... and compute the gradients for them
  float gradient1 = luma1 - luma_center;
  float gradient2 = luma2 - luma_center;

  bool is_1_steepest = abs(gradient1) >= abs(gradient2);
  float gradient_scaled = 0.25f * max(abs(gradient1), abs(gradient2));

  // Figure what the step should be (1 texel in what direction)
  float step_length = is_horizontal ? texel_offset.y : texel_offset.x;

  float luma_local_average = 0.0f;

  if (is_1_steepest) {
    step_length = -step_length;
    luma_local_average = 0.5f * (luma1 + luma_center);
  } else {
    luma_local_average = 0.5f * (luma2 + luma_center);
  }

  vec2 current_uv = texture_coordinate;
  if (is_horizontal) {
    current_uv.y += step_length * 0.5f;
  } else {
    current_uv.x += step_length * 0.5f;
  }

  vec2 offset = is_horizontal ? vec2(texel_offset.x, 0.0f) : vec2(0.0f, texel_offset.y);
  vec2 uv1 = current_uv - offset;
  vec2 uv2 = current_uv + offset;

  float luma_end1 = rgb2luma(texture(screen_texture, uv1).rgb) - luma_local_average;
  float luma_end2 = rgb2luma(texture(screen_texture, uv2).rgb) - luma_local_average;

  bool reached1 = abs(luma_end1) >= gradient_scaled;
  bool reached2 = abs(luma_end2) >= gradient_scaled;
  bool reached_both = reached1 && reached2;

  if (!reached1) {
    uv1 -= offset;
  }
  if (!reached2) {
    uv2 += offset;
  }

  if (!reached_both) {
    for (int i=2; i<ITERATIONS; i++) {
      if (!reached1) {
        luma_end1 = rgb2luma(texture(screen_texture, uv1).rgb) - luma_local_average;
      }
      if (!reached2) {
        luma_end2 = rgb2luma(texture(screen_texture, uv2).rgb) - luma_local_average;
      }
      reached1 = abs(luma_end1) >= gradient_scaled;
      reached2 = abs(luma_end2) >= gradient_scaled;
      reached_both = reached1 && reached2;

      if (!reached1) {
        uv1 -= offset;
      }
      if (!reached2) {
        uv2 += offset;
      }

      if (reached_both) break;
    }
  }

  float distance1 = is_horizontal ? (texture_coordinate.x - uv1.x) : (texture_coordinate.y - uv1.y);
  float distance2 = is_horizontal ? (uv2.x - texture_coordinate.x) : (uv2.y - texture_coordinate.y);

  bool is_direction1 = distance1 < distance2;
  float distance_final = min(distance1, distance2);

  float edge_thickness = distance1 + distance2;

  float pixel_offset = 0.5f - distance_final / edge_thickness;

  bool is_luma_center_smaller = luma_center < luma_local_average;
  bool correct_variation = ((is_direction1 ? luma_end1 : luma_end2) < 0.0f) != is_luma_center_smaller;

  float final_offset = correct_variation ? pixel_offset : 0.0f;

  float luma_average = (1.0f/12.0f) * (2.0f * (luma_down_up + luma_left_right) + luma_left_corners + luma_right_corners);
  float sub_pixel_offset1 = clamp(abs(luma_average - luma_center)/luma_range, 0.0f, 1.0f);
  float sub_pixel_offset2 = (3.0f - 2.0f*sub_pixel_offset1) * sub_pixel_offset1 * sub_pixel_offset1;
  float sub_pixel_offset_final = sub_pixel_offset2*sub_pixel_offset2*SUBPIXEL_QUALITY;

  final_offset = max(final_offset, sub_pixel_offset_final);

  vec2 final_uv = texture_coordinate;
  if (is_horizontal) {
    final_uv.y += final_offset * step_length;
  } else {
    final_uv.x += final_offset * step_length;
  }

  vec3 final_color = texture(screen_texture, final_uv).rgb;

  if (final_offset > 1.0f) final_color = vec3(0.0f, 1.0f, 0.0f);

  frag_color = vec4(final_color, 1.0f);
}
