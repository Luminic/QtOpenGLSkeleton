#version 450

layout (location = 0) out vec4 frag_color;
layout (location = 1) out vec4 volumetric_color;

in VS_OUT {
	vec3 fragment_position;
	vec2 texture_coordinate;
	vec3 normal;
} fs_in;

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

	vec3 color;
	float ambient;
  float diffuse;
	float specular;
  float roughness; // Should be in range 0.1 - 1.0 (shininess is calculated as 2^(roughness*10))
  float metalness;
};

struct DirLight {
	vec3 direction;

	vec3 color;
	float ambient;
	float diffuse;
	float specular;

	sampler2D shadow_map;
	mat4 light_space;
};

struct Light {
	vec3 position;

	vec3 color;
	float ambient;
	float diffuse;
	float specular;

	float constant;
	float linear;
	float quadratic;

	int samples;
	float sample_radius;

	samplerCube shadow_cubemap;
};

uniform samplerCube skybox;

uniform Material material;

uniform int nr_dirlights;
uniform DirLight dirlights[2];

uniform int nr_lights;
uniform Light lights[2];

uniform vec3 camera_position;

float in_dirlight_shadow(DirLight dirlight, bool use_pcf) {
	vec4 position_light_space = dirlight.light_space * vec4(fs_in.fragment_position, 1.0f);
	vec3 projected_coordinates = position_light_space.xyz / position_light_space.w;
	projected_coordinates = projected_coordinates * 0.5f + 0.5f;
	if (projected_coordinates.z > 1.0f) return 0.0f;

	float current_depth = max(0.001f,projected_coordinates.z); // If a fragment has a depth of <=0.0f, then it will always be lit... even if its behind the light source

	if (use_pcf) {
		float shadow = 0.0f;
		vec2 texel_size = 1.0f / textureSize(dirlight.shadow_map, 0);
		for (int x=-1; x<=1; x++) {
			for (int y=-1; y<=1; y++) {
				float pcf_depth = texture(dirlight.shadow_map, projected_coordinates.xy+vec2(x,y)*texel_size).r;
				shadow += current_depth > pcf_depth ? 1.0f : 0.0f;
			}
		}
		shadow /= 9.0f;

		return shadow;
	} else {
		float closest_depth = texture(dirlight.shadow_map, projected_coordinates.xy).r;
		return current_depth > closest_depth ? 1.0f : 0.0f;
	}
}

vec3 dirlight_vis(DirLight dirlight) {
	vec4 position_light_space = dirlight.light_space * vec4(fs_in.fragment_position, 1.0f);
	vec3 projected_coordinates = position_light_space.xyz / position_light_space.w;
	projected_coordinates = projected_coordinates * 0.5f + 0.5f;
	// if (projected_coordinates.z > 1.0f) return vec3(1.0f,0.0f,0.0f);
	// float closest_depth = texture(dirlight.shadow_map, projected_coordinates.xy).r;
	return projected_coordinates;
}

vec3 sample_offset_directions[26] = vec3[](
	vec3( 0,  0,  0), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
	vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
	vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
	vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
	vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1),
	vec3( 0,  0,  1), vec3( 0,  0, -1), vec3( 0,  1,  0), vec3( 1,  0,  0),
	vec3( 0, -1,  0), vec3(-1,  0,  0)
);

float in_pointlight_shadow(Light pointlight, vec3 position, bool use_pcf) {
	vec3 position_to_light = position - pointlight.position;
	float current_depth = length(position_to_light);
	float shadow = 0.0f;

	for (int i=0; i<pointlight.samples; i++) {
		float closest_depth = texture(pointlight.shadow_cubemap, position_to_light+sample_offset_directions[i]*pointlight.sample_radius).r;
		closest_depth *= 45.0f;
		shadow += current_depth > closest_depth ? 1.0f : 0.0f;
	}
	shadow /= pointlight.samples;

	return shadow;
}

float compute_scattering(float cos_theta, float G) {
	// This is the Henyey Greenstein Phase Function
	// Equation from https://www.astro.umd.edu/~jph/HG_note.pdf
	float result = 1 - G*G;
	result /= 4 * 3.1415926535f * pow(1 + G*G - 2*G*cos_theta, 1.5f);
	return result;
}

vec3 calculate_dirlight(DirLight dirlight, vec3 ambient_color, vec3 albedo_color, vec3 specular_color, float shininess, vec3 fragment_normal, vec3 camera_direction) {

	vec3 dirlight_direction = normalize(dirlight.direction);
  vec3 dirlight_halfway_direction = normalize(dirlight_direction+camera_direction);

	// Ambient lighting
	vec3 ambient = ambient_color * dirlight.ambient * dirlight.color;
	// Diffuse Lighting
	float diff = max(dot(fragment_normal, dirlight_direction), 0.0f);
	vec3 diffuse = diff * albedo_color * dirlight.diffuse * dirlight.color;
	// Specular Lighting
	float spec = pow(max(dot(fragment_normal, dirlight_halfway_direction), 0.0), shininess);
	vec3 specular = spec * specular_color * dirlight.specular * dirlight.color;

	// Total Sunlight
	return ambient + (1.0f-in_dirlight_shadow(dirlight, true))*(diffuse + specular);
	// return dirlight_vis(dirlight);
}

vec3 calculate_pointlight(Light light, vec3 ambient_color, vec3 albedo_color, vec3 specular_color, float shininess, vec3 fragment_normal, vec3 camera_direction) {

	vec3 light_direction = normalize(light.position-fs_in.fragment_position);
  vec3 halfway_direction = normalize(light_direction+camera_direction);

	// Ambient lighting
	vec3 ambient = ambient_color * light.ambient * light.color;
	// Diffuse Lighting
	float diff = max(dot(fragment_normal, light_direction), 0.0f);
	vec3 diffuse = diff * albedo_color * light.diffuse * light.color;
	// Specular Lighting
	float spec = pow(max(dot(fragment_normal, halfway_direction), 0.0), shininess);
	vec3 specular = spec * specular_color * light.specular * light.color;

	// Falloff
	float distance = length(fs_in.fragment_position-light.position);
	float falloff = 1.0f / (light.constant + light.linear*distance + light.quadratic*distance*distance);

	// Total Point Lighting
	return (ambient + (1.0f-in_pointlight_shadow(light, fs_in.fragment_position, false))*(diffuse + specular)) * falloff;
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

  vec3 diffuse = vec3(0.0f);
  if (material.number_albedo_maps == 0) {
  	diffuse = material.diffuse * material.color;
  } else {
    for (int i=0; i<material.number_albedo_maps; i++) {
      diffuse += texture(material.albedo_map[i], fs_in.texture_coordinate).rgb;
    }
    diffuse *= material.diffuse * material.color;
  }

  vec3 ambient = diffuse * material.ambient;
  if (material.use_ambient_occlusion_map) {
    //ambient *= texture(material.ambient_occlusion_map, fs_in.texture_coordinate).rgb;
  }

  vec3 specular = vec3(material.specular);
  specular *= pow(roughness, 2);

  vec3 metal_tint = diffuse;
  if (material.metalness >= 0.9f) {
    specular *= normalize(diffuse) * 1.73;
    diffuse *= 1.0f-roughness;
  }

	vec3 lighting_color = vec3(0.0f);

	for (int i=0; i<nr_dirlights; i++) {
		lighting_color += calculate_dirlight(dirlights[i], ambient, diffuse, specular, shininess, fragment_normal, camera_direction);
	}

	for (int i=0; i<nr_lights; i++) {
		lighting_color += calculate_pointlight(lights[i], ambient, diffuse, specular, shininess, fragment_normal, camera_direction);
	}

	vec3 reflection_color = vec3(0.0f);
	if (material.metalness >= 0.9f) {
		vec3 R = reflect(-camera_direction, fragment_normal);
		reflection_color = texture(skybox, R).rgb * roughness * metal_tint;
	}

	vec3 total_color = vec3(lighting_color+reflection_color);
	total_color = clamp(total_color, 0.0f.xxx, 1.0f.xxx);

	// Final result
  frag_color = vec4(total_color, 1.0f);//vec4(mix(total_color,total_scattering.rgb,total_scattering.a), 1.0f);
	volumetric_color = frag_color;
}
