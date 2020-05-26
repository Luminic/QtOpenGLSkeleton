#version 450

layout (location = 0) out vec4 frag_color;

in VS_OUT {
	vec3 fragment_position;
	vec2 texture_coordinate;
	vec3 normal;
} fs_in;

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

	sampler2D opacity_map; // Only exists in the full transparency & partial transparency shaders
	bool use_opacity_map;
	float opacity;

	vec3 color;
	float ambient;
  float diffuse;
	float specular;
  float roughness; // Should be in range 0.1 - 1.0 (shininess is calculated as 2^(roughness*10))
  float metalness;
};

#mypreprocessor include "../shader_components/light_structs.glsl"

uniform samplerCube skybox;
uniform float skybox_multiplier;

uniform Material material;

uniform int nr_dirlights;
uniform DirLight dirlights[MAX_NR_DIRLIGHTS];

uniform int nr_lights;
uniform Light lights[MAX_NR_LIGHTS];

uniform vec3 camera_position;

#define MAXIMUM_BRIGHTNESS 50.0f

#mypreprocessor include "../shader_components/shadow_functions.glsl"

float compute_scattering(float cos_theta, float G) {
	// This is the Henyey Greenstein Phase Function
	// Equation from https://www.astro.umd.edu/~jph/HG_note.pdf
	float result = 1 - G*G;
	result /= 4 * 3.1415926535f * pow(1 + G*G - 2*G*cos_theta, 1.5f);
	return result;
}

#mypreprocessor include "../shader_components/light_calculation_functions.glsl"

float linear_depth(float depth) {
  float near = 0.1;
  float far  = 100.0;
  float z = 2 * depth - 1;
  return (2.0 * near * far) / (far + near - z * (far - near)) / far;
}

void main() {
	float opacity = material.opacity;
	if (material.use_opacity_map) {
		opacity *= texture(material.opacity_map, fs_in.texture_coordinate).a;
	}
	if (opacity <= 0.05f) {
		discard;
	}

	vec3 fragment_normal = normalize(fs_in.normal);
	vec3 camera_direction = normalize(camera_position - fs_in.fragment_position);

  // Get the colors for everything

  float roughness = material.roughness;
  if (material.use_roughness_map) {
    roughness *= length(texture(material.roughness_map, fs_in.texture_coordinate).rgb)/1.73f;
	}

  float shininess = pow(2,(roughness)*10);

  float metalness = material.metalness;
  if (material.use_metalness_map) {
    metalness *= length(texture(material.metalness_map, fs_in.texture_coordinate).rgb)/1.73f;
	}

  vec3 diffuse_color = material.color;
	if (material.use_albedo_map) {
		diffuse_color *= texture(material.albedo_map, fs_in.texture_coordinate).rgb;
	}
	vec3 diffuse = material.diffuse * diffuse_color;

  vec3 specular = vec3(material.specular);
  specular *= pow(roughness, 2);

  vec3 metal_tint = diffuse_color;
  if (metalness >= 0.9f) {
    specular *= normalize(diffuse) * 1.73;
    diffuse *= 1.0f-roughness;
  }

	vec3 ambient = diffuse_color * material.ambient;
  if (material.use_ambient_occlusion_map) {
    ambient *= texture(material.ambient_occlusion_map, fs_in.texture_coordinate).rgb;
  }

	vec3 lighting_color = vec3(0.0f);

	for (int i=0; i<nr_dirlights; i++) {
		lighting_color += calculate_dirlight(dirlights[i], ambient, diffuse, specular, shininess, fragment_normal, camera_direction, fs_in.fragment_position);
	}

	for (int i=0; i<nr_lights; i++) {
		lighting_color += calculate_pointlight(lights[i], ambient, diffuse, specular, shininess, fragment_normal, camera_direction, fs_in.fragment_position);
	}

	vec3 reflection_color = vec3(0.0f);
	if (metalness >= 0.9f) {
		vec3 R = reflect(-camera_direction, fragment_normal);
		reflection_color = texture(skybox, R).rgb * skybox_multiplier * roughness * metal_tint;
	}

	vec3 total_color = vec3(lighting_color+reflection_color);
	total_color = clamp(total_color, 0.0f.xxx, MAXIMUM_BRIGHTNESS.xxx);

	// Final result
  frag_color = vec4(total_color, 1.0f);//vec4(mix(total_color,total_scattering.rgb,total_scattering.a), 1.0f);
}
