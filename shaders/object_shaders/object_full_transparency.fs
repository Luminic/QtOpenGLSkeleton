#version 450

#define TRANSPARENCY_FULL 1
#define TYPE_OBJECT 1

layout (location = 0) out vec4 frag_color;

in VS_OUT {
	vec3 fragment_position;
	vec2 texture_coordinate;
	vec3 normal;
} fs_in;

#mypreprocessor include "../shader_components/material_struct.glsl"

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

#mypreprocessor include "../shader_components/light_calculation_functions.glsl"

void main() {
	if (material.simple) {
		frag_color = vec4(material.color, 1.0f);
		return;
	}
	
	float opacity = material.opacity;
	if (material.use_opacity_map) {
		opacity *= texture(material.opacity_map, fs_in.texture_coordinate).a;
	}
	if (opacity <= 0.05f) {
		discard;
	}

	vec3 fragment_normal = normalize(fs_in.normal);
	vec3 camera_direction = normalize(camera_position - fs_in.fragment_position);

  // Get the material properties
	float roughness;
	float shininess;
	float metalness;
	vec3 color;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 metal_tint;
	#mypreprocessor include "../shader_components/material_properties_calculation.glsl"

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
