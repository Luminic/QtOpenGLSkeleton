#version 420

out vec4 frag_color;

in vec2 texture_coordinate;

uniform sampler2D screen_texture;

uniform int display_type;

void main() {
  switch (display_type) {
    case 1:
      frag_color = vec4(texture(screen_texture, texture_coordinate).rrr, 1.0f);
      break;
    default:
      frag_color = texture(screen_texture, texture_coordinate);
      break;
  }
  //frag_color = vec4(vec3(display_type), 1.0f);
  //frag_color = vec4(vec3(texture(screen_texture, texture_coordinate).r), 1.0f);
}

/*void main() {
  frag_color = vec4(texture_coordinate.xyx, 1.0f);
}*/

/*void main() {
  vec3 col = texture(screen_texture, texture_coordinate).rgb;
  float tr = 0.393*col.r + 0.769*col.g + 0.189*col.b;
  float tg = 0.349*col.r + 0.686*col.g + 0.168*col.b;
  float tb = 0.272*col.r + 0.534*col.g + 0.131*col.b;
  frag_color = vec4(tr, tg, tb, 1.0f);
}*/


/*
const float offset = 1.0f / 300.0f;

void main() {
  vec2 offsets[9] = vec2[](
    vec2(-offset,  offset), // top-left
    vec2( 0.0f,    offset), // top-center
    vec2( offset,  offset), // top-right
    vec2(-offset,  0.0f),   // center-left
    vec2( 0.0f,    0.0f),   // center-center
    vec2( offset,  0.0f),   // center-right
    vec2(-offset, -offset), // bottom-left
    vec2( 0.0f,   -offset), // bottom-center
    vec2( offset, -offset)  // bottom-right
  );

  float kernel[9] = float[](
    -1, -1, -1,
    -1,  9, -1,
    -1, -1, -1
  );

  float kernel[9] = float[](
    1.0/16, 2.0/16, 1.0/16,
    2.0/16, 4.0/16, 2.0/16,
    1.0/16, 2.0/16, 1.0/16
  );

  float kernel[9] = float[](
    1, 1, 1,
    1,-9, 1,
    1, 1, 1
  );

  vec3 col = vec3(0.0f);
  for (int i=0; i<9; i++) {
    vec3 texture_at = texture(screen_texture, texture_coordinate+offsets[i]).rgb;
    col += texture_at * kernel[i];
  }
  //col.r *= 0.8;
  col.g += 0.2;
  //col.b *= 0.8;

  frag_color = vec4(col, 1.0f);
}*/
