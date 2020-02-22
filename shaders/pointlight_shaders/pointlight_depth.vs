#version 450

layout(location=0) in vec3 vertex_position;
layout(location=2) in vec2 vertex_texture_coordinate;

uniform mat4 model;

out vec2 vert_texture_coordinate;

void main() {
	vert_texture_coordinate = vertex_texture_coordinate;
	gl_Position = model * vec4(vertex_position, 1.0f);
}
