#version 420

layout(location=0) in vec3 vertex_position;

uniform mat4 light_space;
uniform mat4 model;

void main() {
	gl_Position = light_space * model * vec4(vertex_position, 1.0f);
}
