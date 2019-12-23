#version 420

in VS_to_FS {
	vec3 fragment_position;
	vec2 texture_coordinate;
	vec3 normal;
	vec4 fragment_light_space;
} fs_in;

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
	sampler2D shadow_map;
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
uniform mat4 light_space;

uniform bool use_volumetric_lighting;
uniform float volumetric_multiplier;
uniform float volumetric_offset;
uniform int steps;
uniform float henyey_greenstein_G_value;

float in_shadow(sampler2D shadow_map, bool use_pcf) {
	vec3 projected_coordinates = fs_in.fragment_light_space.xyz / fs_in.fragment_light_space.w;
	projected_coordinates = projected_coordinates * 0.5f + 0.5f;
	if (projected_coordinates.z > 1.0f) return 0.0f;

	float current_depth = projected_coordinates.z;

	if (use_pcf) {
		float shadow = 0.0f;
		vec2 texel_size = 1.0f / textureSize(sunlight.shadow_map, 0);
		for (int x=-1; x<=1; x++) {
			for (int y=-1; y<=1; y++) {
				float pcf_depth = texture(shadow_map, projected_coordinates.xy+vec2(x,y)*texel_size).r;
				shadow += current_depth > pcf_depth ? 1.0f : 0.0f;
			}
		}
		shadow /= 9.0f;

		return shadow;
	} else {
		float closest_depth = texture(shadow_map, projected_coordinates.xy).r;
		return current_depth > closest_depth ? 1.0f : 0.0f;
	}
}

float in_shadow_fp(sampler2D shadow_map, vec3 position_world_space, bool use_pcf) {
	vec4 position_light_space = light_space * vec4(position_world_space, 1.0f);
	vec3 projected_coordinates = position_light_space.xyz / position_light_space.w;
	projected_coordinates = projected_coordinates * 0.5f + 0.5f;
	if (projected_coordinates.z > 1.0f) return 0.0f;

	float current_depth = projected_coordinates.z;

	if (use_pcf) {
		float shadow = 0.0f;
		vec2 texel_size = 1.0f / textureSize(sunlight.shadow_map, 0);
		for (int x=-1; x<=1; x++) {
			for (int y=-1; y<=1; y++) {
				float pcf_depth = texture(shadow_map, projected_coordinates.xy+vec2(x,y)*texel_size).r;
				shadow += current_depth > pcf_depth ? 1.0f : 0.0f;
			}
		}
		shadow /= 9.0f;

		return shadow;
	} else {
		float closest_depth = texture(shadow_map, projected_coordinates.xy).r;
		return current_depth > closest_depth ? 1.0f : 0.0f;
	}
}

float compute_scattering(float cos_theta, float G) {
	// This is the Henyey Greenstein Phase Function
	// Equation from https://www.astro.umd.edu/~jph/HG_note.pdf
	float result = 1 - G*G;
	result /= 4 * 3.1415926535f * pow(1 + G*G - 2*G*cos_theta, 1.5f);
	return result;
}

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
	return ambient + (1.0f-in_shadow(sunlight.shadow_map, true))*(diffuse + specular);
}

vec3 calculate_pointlight(Light light, vec3 ambient_color, vec3 albedo_color, vec3 specular_color, float shininess, vec3 fragment_normal, vec3 camera_direction) {

	vec3 light_direction = normalize(light.position-fs_in.fragment_position);
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
	float distance = length(fs_in.fragment_position-light.position);
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
	vec3 fragment_normal = normalize(fs_in.normal);
	vec3 camera_direction = normalize(camera_position - fs_in.fragment_position);

  // Get the colors for everything

  float roughness = material.roughness;
  if (material.use_roughness_map)
    roughness *= length(texture(material.roughness_map, fs_in.texture_coordinate).rgb)/1.73f;

  float shininess = pow(2,(roughness)*10);

  float metalness = material.metalness;
  if (material.use_metalness_map)
    metalness *= length(texture(material.metalness_map, fs_in.texture_coordinate).rgb)/1.73f;

  vec3 albedo = vec3(0.0f);
  if (material.number_albedo_maps == 0) {
  albedo = material.albedo;
  } else {
    for (int i=0; i<material.number_albedo_maps; i++) {
      albedo += texture(material.albedo_map[i], fs_in.texture_coordinate).rgb;
    }
    albedo *= material.albedo;
  }

  vec3 ambient = albedo * material.ambient;
  if (material.use_ambient_occlusion_map) {
    //ambient *= texture(material.ambient_occlusion_map, fs_in.texture_coordinate).rgb;
  }

  vec3 specular = vec3(1.0f);
  specular *= pow(roughness, 2);

  vec3 metal_tint = albedo;
  if (material.metalness >= 0.9f) {
    specular *= normalize(albedo) * 1.73;
    albedo *= 1.0f-roughness;
  }

	vec3 lighting_color = calculate_sunlight(sunlight, ambient, albedo, specular, shininess, fragment_normal, camera_direction);
	/*for (int i=0; i<4; i++) {
		lighting_color += calculate_pointlight(light[i], ambient, albedo, specular, shininess, fragment_normal, camera_direction);
	}*/

	float total_scattering = 0.0f;
	if (use_volumetric_lighting) {
		vec3 difference = fs_in.fragment_position-camera_position;
		float distance = length(difference);

		vec3 current_pos = camera_position;
		for (int i=0; i<steps; i++) {
			current_pos += difference/float(steps+1);
			float scattering = 1.0f-in_shadow_fp(sunlight.shadow_map, current_pos, true);
			scattering *= compute_scattering(dot(-camera_direction, normalize(sunlight.direction)), henyey_greenstein_G_value);
			total_scattering += scattering;
		}
		total_scattering = max(total_scattering/steps*volumetric_multiplier*pow(distance, 1/2) + volumetric_offset, 0.0f);
	}

	//frag_color = vec4(vec3(total_scattering), 1.0f);


  // Gamma Correction
  float gamma = 2.2f;
	total_scattering = pow(total_scattering, 1.0/gamma);
  lighting_color = pow(lighting_color, vec3(1.0/gamma));
  metal_tint = pow(metal_tint, vec3(1.0/gamma));

  vec3 reflection_color = vec3(0.0f);
  if (material.metalness >= 0.9f) {
    vec3 R = reflect(-camera_direction, fragment_normal);
    reflection_color = texture(skybox, R).rgb * roughness * metal_tint;
  }

	vec3 total_color = vec3(lighting_color+reflection_color);

	// Final result
  //frag_color = vec4(vec3(max(dot(fragment_normal,camera_direction),0.0f)),1.0f); // Use to check face orientation
  //frag_color = vec4(vec3(result*(1-linear_depth(gl_FragCoord.z))+linear_depth(gl_FragCoord.z)), 1.0f); // Use to check linear depth
  frag_color = vec4(mix(total_color,vec3(1.0f),total_scattering), 1.0f);
	//frag_color = vec4(vec3(distance/10.0f), 1.0f);
}
