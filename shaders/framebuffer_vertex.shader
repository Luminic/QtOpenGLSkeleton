#version 420

layout(location=0) in vec2 vertex_position;
layout(location=1) in vec2 vertex_texture_coordinate;

out vec2 texture_coordinate;

void main() {
  gl_Position = vec4(vertex_position.x, vertex_position.y, 0.0, 1.0);
  texture_coordinate = vertex_texture_coordinate;
}
