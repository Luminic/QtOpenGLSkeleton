#version 450

layout (depth_greater) out float gl_FragDepth;

in vec2 texture_coordinate;

struct Material {
  sampler2D opacity_map; // Only exists in the full transparency & partial transparency shaders
  bool use_opacity_map;
  float opacity;
};

uniform Material material;

void main() {
  float opacity = material.opacity;
	if (material.use_opacity_map) {
		opacity *= texture(material.opacity_map, texture_coordinate).a;
	}
	if (opacity <= 0.05f) {
		discard;
	}

  gl_FragDepth = gl_FragCoord.z;
  gl_FragDepth += gl_FrontFacing ? 0.002 : 0.0;
}
