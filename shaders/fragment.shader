#version 420

in vec2 texture_coordinate;
in vec3 normal;
in vec3 fragment_position;

out vec4 frag_color;

struct Material {
  // Should be 0 and false by default
  int number_albedo_maps;
  bool use_ambient_occlusion_map;
  bool use_roughness_map; // Inverted (white is smooth and black is rough)
  bool use_metalness_map;

  // These should be uninitialized and unused by default
  sampler2D albedo_map[1];
  //sampler2D ambient_occlusion_map;
  sampler2D roughness_map;
  sampler2D metalness_map;

  // Should be vec3(1.0f), 1.0f, 1.0f, & 0.0f by default
  vec3 albedo;
  vec3 ambient;
  float roughness; // Should be in range 0.1 - 1.0 (shininess is calculated as 2^(roughness*10))
  float metalness;
};

struct Sunlight {
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Light {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

uniform samplerCube skybox;
uniform int test;

uniform Material material;
uniform Sunlight sunlight;
uniform Light light[4];

uniform vec3 camera_position;

vec3 calculate_sunlight(Sunlight sunlight, vec3 ambient_color, vec3 albedo_color, vec3 specular_color, float shininess, vec3 fragment_normal, vec3 camera_direction) {

	vec3 sunlight_direction = normalize(sunlight.direction);
  vec3 sunlight_halfway_direction = normalize(sunlight_direction+camera_direction);

	// Ambient lighting
	vec3 ambient = ambient_color * sunlight.ambient;
	// Diffuse Lighting
	float diff = max(dot(fragment_normal, sunlight_direction), 0.0f);
	vec3 diffuse = diff * albedo_color * sunlight.diffuse;
	// Specular Lighting
	float spec = pow(max(dot(fragment_normal, sunlight_halfway_direction), 0.0), shininess);
	vec3 specular = spec * specular_color * sunlight.specular;

	// Total Sunlight
	return ambient + diffuse + specular;
}

vec3 calculate_pointlight(Light light, vec3 ambient_color, vec3 albedo_color, vec3 specular_color, float shininess, vec3 fragment_normal, vec3 fragment_position, vec3 camera_direction) {

	vec3 light_direction = normalize(light.position-fragment_position);
  vec3 halfway_direction = normalize(light_direction+camera_direction);

	// Ambient lighting
	vec3 ambient = ambient_color * light.ambient;
	// Diffuse Lighting
	float diff = max(dot(fragment_normal, light_direction), 0.0f);
	vec3 diffuse = diff * albedo_color * light.diffuse;
	// Specular Lighting
	float spec = pow(max(dot(fragment_normal, halfway_direction), 0.0), shininess);
	vec3 specular = spec * specular_color * light.specular;

	// Falloff
	float distance = length(fragment_position-light.position);
	float falloff = 1.0f / (light.constant + light.linear*distance + light.quadratic*distance*distance);

	// Total Point Lighting
	return (ambient + diffuse + specular) * falloff;
}

float linear_depth(float depth) {
  float near = 0.1;
  float far  = 100.0;
  float z = 2 * depth - 1;
  return (2.0 * near * far) / (far + near - z * (far - near)) / far;
}

void main() {
	vec3 fragment_normal = normalize(normal);
	vec3 camera_direction = normalize(camera_position - fragment_position);

  // Get the colors for everything

  float roughness = material.roughness;
  if (material.use_roughness_map)
    roughness *= length(texture(material.roughness_map, texture_coordinate).rgb)/1.73f;

  float shininess = pow(2,(roughness)*10);

  float metalness = material.metalness;
  if (material.use_metalness_map)
    metalness *= length(texture(material.metalness_map, texture_coordinate).rgb)/1.73f;

  vec3 albedo = vec3(0.0f);
  if (material.number_albedo_maps == 0) {
  albedo = material.albedo;
  } else {
    for (int i=0; i<material.number_albedo_maps; i++) {
      albedo += texture(material.albedo_map[i], texture_coordinate).rgb;
    }
    albedo *= material.albedo;
  }

  vec3 ambient = albedo * material.ambient;
  if (material.use_ambient_occlusion_map) {
    //ambient *= texture(material.ambient_occlusion_map, texture_coordinate).rgb;
  }

  vec3 specular = vec3(1.0f);
  specular *= pow(roughness, 2);

  vec3 metal_tint = albedo;
  if (material.metalness >= 0.9f) {
    specular *= normalize(albedo) * 1.73;
    albedo *= 1.0f-roughness;
  }

	vec3 lighting_color = calculate_sunlight(sunlight, ambient, albedo, specular, shininess, fragment_normal, camera_direction);
	for (int i=0; i<4; i++) {
		lighting_color += calculate_pointlight(light[i], ambient, albedo, specular, shininess, fragment_normal, fragment_position, camera_direction);
	}

  // Gamma Correction
  float gamma = 2.2;
  lighting_color = pow(lighting_color, vec3(1.0/gamma));
  metal_tint = pow(metal_tint, vec3(1.0/gamma));

  vec3 reflection_color = vec3(0.0f);
  if (material.metalness >= 0.9f) {
    vec3 R = reflect(-camera_direction, fragment_normal);
    reflection_color = texture(skybox, R).rgb * roughness * metal_tint;
  }

	// Final result
  //frag_color = vec4(vec3(max(dot(fragment_normal,camera_direction),0.0f)),1.0f); // Use to check face orientation
  //frag_color = vec4(vec3(result*(1-linear_depth(gl_FragCoord.z))+linear_depth(gl_FragCoord.z)), 1.0f); // Use to check linear depth
  frag_color = vec4(vec3(lighting_color+reflection_color), 1.0f);
}
