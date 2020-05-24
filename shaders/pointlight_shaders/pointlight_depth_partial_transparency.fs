#version 450

in vec4 fragment_position;
in vec2 texture_coordinate;

struct Material {
  sampler2D opacity_map; // Only exists in the full transparency & partial transparency shaders
  bool use_opacity_map;
  float opacity;
};

uniform Material material;
uniform vec3 pointlight_position;
uniform float far_plane;

void main() {
  float opacity = material.opacity;
	if (material.use_opacity_map) {
		opacity *= texture(material.opacity_map, texture_coordinate).a;
	}
	if (opacity <= 0.05f) {
		discard;
	}

  float light_distance = length(fragment_position.xyz - pointlight_position);
  light_distance /= far_plane;
  gl_FragDepth = light_distance;
  gl_FragDepth += gl_FrontFacing ? 0.002 : 0.0;
}
