#version 420

layout(location=0) in vec3 vertex_position;

out vec3 texture_coordinate;

uniform mat4 projection;
uniform mat4 view;

void main() {
  texture_coordinate = vertex_position;
  gl_Position = projection * view * vec4(vertex_position, 1.0);
}
