#version 420

in vec2 texture_coordinate;
in vec3 normal;
in vec3 fragment_position;

out vec4 frag_color;

struct Material {
  float metalness; // Default should be 0 (non-metallic)

  // Should be 0 and false by default
  int number_albedo_maps;
  bool use_ambient_occlusion_map;
  bool use_specular_map;

  // These should be uninitialized and unused by default
  sampler2D albedo_map[1];
  sampler2D ambient_occlusion_map;
  // If metalness is 0.0f this is a specular map. Otherwise, it is a metalness map
  sampler2D specular_map;

  // Should be 1.0f by default
  vec3 albedo;
  vec3 ambient;
  float specularity;

  float shininess; // Default should be 64.0f
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

uniform Material material;
uniform Sunlight sunlight;
uniform Light light[4];

uniform vec3 camera_position;

vec3 calculate_sunlight(Sunlight sunlight, vec3 ambient_color, vec3 albedo_color, vec3 specular_color, vec3 fragment_normal, vec3 camera_direction) {

	vec3 sunlight_direction = normalize(sunlight.direction);
  vec3 sunlight_halfway_direction = normalize(sunlight_direction+camera_direction);

	// Ambient lighting
	vec3 ambient = ambient_color * sunlight.ambient;
	// Diffuse Lighting
	float diff = max(dot(fragment_normal, sunlight_direction), 0.0f);
	vec3 diffuse = diff * albedo_color * sunlight.diffuse;
	// Specular Lighting
	float spec = pow(max(dot(fragment_normal, sunlight_halfway_direction), 0.0), material.shininess);
	vec3 specular = spec * specular_color * sunlight.specular;

	// Total Sunlight
	return ambient + diffuse + specular;
}

vec3 calculate_pointlight(Light light, vec3 ambient_color, vec3 albedo_color, vec3 specular_color, vec3 fragment_normal, vec3 fragment_position, vec3 camera_direction) {

	vec3 light_direction = normalize(light.position-fragment_position);
  vec3 halfway_direction = normalize(light_direction+camera_direction);

	// Ambient lighting
	vec3 ambient = ambient_color * light.ambient;
	// Diffuse Lighting
	float diff = max(dot(fragment_normal, light_direction), 0.0f);
	vec3 diffuse = diff * albedo_color * light.diffuse;
	// Specular Lighting
	float spec = pow(max(dot(fragment_normal, halfway_direction), 0.0), material.shininess);
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
  if (material.use_ambient_occlusion_map){}
    ambient *= texture(material.ambient_occlusion_map, texture_coordinate).rgb;

  vec3 specular = vec3(material.specularity);

  // Scale the albedo and specular values based on how metallic the object is
  if (material.use_specular_map) {
    if (material.metalness == 0.0f) {
      specular *= texture(material.specular_map, texture_coordinate).rgb;
    } else {
      // The color of specular reflections on metal objects is the albedo color
      specular *= material.metalness * texture(material.specular_map, texture_coordinate).rgb * normalize(albedo);
      albedo *= 1-(material.metalness * texture(material.specular_map, texture_coordinate).rgb);
    }
  } else {
    specular *= material.metalness * normalize(albedo); // Specular reflections on metal objects is the albedo color
    albedo *= 1-material.metalness;
  }

	vec3 result = calculate_sunlight(sunlight, ambient, albedo, specular, fragment_normal, camera_direction);
	for (int i=0; i<4; i++) {
		result += calculate_pointlight(light[i], ambient, albedo, specular, fragment_normal, fragment_position, camera_direction);
	}

  // Gamma Correction
  float gamma = 2.2;
  result = pow(result, vec3(1.0/gamma));

	// Final result
  //frag_color = vec4(vec3(max(dot(fragment_normal,camera_direction),0.0f)),1.0f); // Use to check face orientation
  //frag_color = vec4(vec3(result*(1-linear_depth(gl_FragCoord.z))+linear_depth(gl_FragCoord.z)), 1.0f); // Use to check linear depth
  frag_color = vec4(result,1.0f);

  /*vec3 R = reflect(-camera_direction, fragment_normal);
  frag_color = texture(skybox, R);*/
}
