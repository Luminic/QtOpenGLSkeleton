#version 450

layout (depth_greater) out float gl_FragDepth;

in vec2 texture_coordinate;

struct Material {
  sampler2D opacity_map; // Only exists in the full transparency & partial transparency shaders
};

uniform Material material;

void main() {
  if (texture(material.opacity_map, texture_coordinate).a <= 0.05f) {
		discard;
	}

  gl_FragDepth = gl_FragCoord.z;
  gl_FragDepth += gl_FrontFacing ? 0.002 : 0.0;
}
