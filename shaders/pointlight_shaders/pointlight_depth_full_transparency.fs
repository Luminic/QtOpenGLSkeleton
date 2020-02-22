#version 450

in vec4 fragment_position;
in vec2 texture_coordinate;

struct Material {
  sampler2D opacity_map; // Only exists in the full transparency & partial transparency shaders
};

uniform Material material;
uniform vec3 pointlight_position;
uniform float far_plane;

void main() {
  if (texture(material.opacity_map, texture_coordinate).a <= 0.05f) {
		discard;
	}

  float light_distance = length(fragment_position.xyz - pointlight_position);
  light_distance /= far_plane;
  gl_FragDepth = light_distance;
  gl_FragDepth += gl_FrontFacing ? 0.002 : 0.0;
}
