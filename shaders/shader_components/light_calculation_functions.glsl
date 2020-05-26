#ifndef LIGHT_CALCULATION_FUNCTIONS_GLSL
#define LIGHT_CALCULATION_FUNCTIONS_GLSL

#mypreprocessor include "light_structs.glsl"
#mypreprocessor include "shadow_functions.glsl"

vec3 calculate_dirlight(DirLight dirlight, vec3 ambient_color, vec3 albedo_color, vec3 specular_color, float shininess, vec3 fragment_normal, vec3 camera_direction, vec3 fragment_position) {

	vec3 dirlight_direction = normalize(dirlight.direction);
  vec3 dirlight_halfway_direction = normalize(dirlight_direction+camera_direction);
	fragment_normal = fragment_normal * sign(dot(fragment_normal, camera_direction));

	// Ambient lighting
	vec3 ambient = ambient_color * dirlight.ambient * dirlight.color;
	// Diffuse Lighting
	float diff = max(dot(fragment_normal, dirlight_direction), 0.0f);
	vec3 diffuse = diff * albedo_color * dirlight.diffuse * dirlight.color;
	// Specular Lighting
	float spec = pow(max(dot(fragment_normal, dirlight_halfway_direction), 0.0), shininess);
	vec3 specular = spec * specular_color * dirlight.specular * dirlight.color;

	// Total Sunlight
	return ambient + (1.0f-in_dirlight_shadow(dirlight, fragment_position, true))*(diffuse + specular);
	// return dirlight_vis(dirlight);
}

vec3 calculate_pointlight(Light light, vec3 ambient_color, vec3 albedo_color, vec3 specular_color, float shininess, vec3 fragment_normal, vec3 camera_direction, vec3 fragment_position) {

	vec3 light_direction = normalize(light.position-fragment_position);
  vec3 halfway_direction = normalize(light_direction+camera_direction);
	fragment_normal = fragment_normal * sign(dot(fragment_normal, camera_direction));

	// Ambient lighting
	vec3 ambient = ambient_color * light.ambient * light.color;
	// Diffuse Lighting
	float diff = max(dot(fragment_normal, light_direction), 0.0f);
	vec3 diffuse = diff * albedo_color * light.diffuse * light.color;
	// Specular Lighting
	float spec = pow(max(dot(fragment_normal, halfway_direction), 0.0), shininess);
	vec3 specular = spec * specular_color * light.specular * light.color;

	// Falloff
	float distance = length(fragment_position-light.position);
	float falloff = 1.0f / (light.constant + light.linear*distance + light.quadratic*distance*distance);

	// Total Point Lighting
	return (ambient + (1.0f-in_pointlight_shadow(light, fragment_position, false))*(diffuse + specular)) * falloff;
}

#endif
