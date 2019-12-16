#version 420

layout(location=0) in vec3 vertex_position;
layout(location=1) in vec3 vertex_normal;
layout(location=2) in vec2 vertex_texture_coordinate;

out vec2 texture_coordinate;
out vec3 normal;
out vec3 fragment_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
	fragment_position = vec3(model * vec4(vertex_position, 1.0f));
	gl_Position = projection * view * vec4(fragment_position, 1.0);
	texture_coordinate = vertex_texture_coordinate;
	normal = mat3(transpose(inverse(model))) * vertex_normal;
}
