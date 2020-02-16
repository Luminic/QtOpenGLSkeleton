#version 450

layout(location=0) in vec3 vertex_position;
layout(location=2) in vec2 vertex_texture_coordinate;

uniform mat4 light_space;
uniform mat4 model;

out vec2 texture_coordinate;

void main() {
	texture_coordinate = vertex_texture_coordinate;
	gl_Position = light_space * model * vec4(vertex_position, 1.0f);
}
