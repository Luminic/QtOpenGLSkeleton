#version 450

#define TRANSPARENCY_OPAQUE 1
#define TYPE_LIGHT 1

in vec4 fragment_position;

uniform vec3 pointlight_position;
uniform float far_plane;

void main() {
  float light_distance = length(fragment_position.xyz - pointlight_position);
  light_distance /= far_plane;
  gl_FragDepth = light_distance;
  gl_FragDepth += gl_FrontFacing ? 0.002 : 0.0;
}
