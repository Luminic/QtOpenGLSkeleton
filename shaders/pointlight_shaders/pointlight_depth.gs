#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 light_spaces[6];

in vec2 vert_texture_coordinate[3];
out vec2 texture_coordinate;
out vec4 fragment_position;

void main() {
  for (int face=0; face<6; face++) {
    gl_Layer = face;
    for (int i=0; i<3; i++) {
      fragment_position = gl_in[i].gl_Position;
      texture_coordinate = vert_texture_coordinate[i];
      gl_Position = light_spaces[face] * fragment_position;
      EmitVertex();
    }
    EndPrimitive();
  }
}
