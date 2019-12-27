#version 420

layout(location=0) out vec4 frag_color;
layout(location=1) out vec4 color;

in vec2 texture_coordinate;

uniform sampler2D image;
uniform bool horizontal;

float weights[5] = float[](0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);

void main() {
  vec2 texel_offset = 1.0f/textureSize(image, 0); // Get the size of a texel
  vec3 result = texture(image, texture_coordinate).rgb * weights[0];

  if (horizontal) {
    texel_offset.y = 0.0f;
  } else {
    texel_offset.x = 0.0f;
  }

  for (int i=1; i<5; i++) {
    result += texture(image, texture_coordinate+texel_offset*i).rgb * weights[i];
    result += texture(image, texture_coordinate-texel_offset*i).rgb * weights[i];
  }

  frag_color = vec4(result, 1.0f);
  color = frag_color;
}
