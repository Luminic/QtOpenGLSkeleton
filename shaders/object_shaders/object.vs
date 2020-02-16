#version 450

layout(location=0) in vec3 vertex_position;
layout(location=1) in vec3 vertex_normal;
layout(location=2) in vec2 vertex_texture_coordinate;

out VS_OUT {
	vec3 fragment_position;
	vec2 texture_coordinate;
	vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	vs_out.fragment_position = vec3(model * vec4(vertex_position, 1.0f));
	vs_out.texture_coordinate = vertex_texture_coordinate;
	vs_out.normal = mat3(transpose(inverse(model))) * vertex_normal;
	gl_Position = projection * view * vec4(vs_out.fragment_position, 1.0);
}
