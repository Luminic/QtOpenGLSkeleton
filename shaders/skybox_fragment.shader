#version 450

out vec4 frag_color;

in vec3 texture_coordinate;

uniform float skybox_multiplier;

uniform samplerCube skybox;
uniform int mode;

void main() {
  if (mode == 1) frag_color = vec4(texture(skybox, texture_coordinate).rrr, 1.0f);
  else frag_color = texture(skybox, texture_coordinate);
  frag_color.rgb *= skybox_multiplier;
}
