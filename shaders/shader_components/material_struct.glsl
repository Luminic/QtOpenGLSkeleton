#ifndef MATERIAL_STRUCT_GLSL
#define MATERIAL_STRUCT_GLSL

struct Material {
  // Should be 0 and false by default
  bool use_albedo_map;
  bool use_ambient_occlusion_map;
  bool use_roughness_map; // Inverted (white is smooth and black is rough)
  bool use_metalness_map;

  // These should be uninitialized and unused by default
  sampler2D albedo_map;
  sampler2D ambient_occlusion_map;
  sampler2D roughness_map;
  sampler2D metalness_map;

  #if defined(TRANSPARENCY_PARTIAL) || defined(TRANSPARENCY_FULL)
  	sampler2D opacity_map; // Only exists in the full transparency & partial transparency shaders
  	bool use_opacity_map;
  	float opacity;
  #endif

	vec3 color;
	float ambient;
  float diffuse;
	float specular;
  float roughness; // Should be in range 0.1 - 1.0 (shininess is calculated as 2^(roughness*10))
  float metalness;

  // If simple, the shader should return vec4(color, 1.0f)
  bool simple;
};

#endif
