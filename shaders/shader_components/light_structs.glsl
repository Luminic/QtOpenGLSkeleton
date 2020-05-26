#ifndef LIGHT_STRUCTS_GLSL
#define LIGHT_STRUCTS_GLSL

#define MAX_NR_DIRLIGHTS 1

struct DirLight {
	vec3 direction;

	vec3 color;
	float ambient;
	float diffuse;
	float specular;

	sampler2D shadow_map;
	mat4 light_space;
};

#define MAX_NR_LIGHTS 1

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

#endif
