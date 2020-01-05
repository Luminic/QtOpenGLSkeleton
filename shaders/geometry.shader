#version 450

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

// Note to self: Do all calculations in worldspace when possible; Camera space hella gey.

in VS_OUT {
	vec3 fragment_position;
	vec2 texture_coordinate;
	vec3 normal;
	vec4 fragment_light_space;
} gs_in[];

out GS_OUT {
  vec3 fragment_position;
	vec2 texture_coordinate;
	vec3 normal;
	vec4 fragment_light_space;
} gs_out;

void vs_out_to_gs_out(int index) {
  gs_out.fragment_position = gs_in[index].fragment_position;
  gs_out.texture_coordinate = gs_in[index].texture_coordinate;
  gs_out.normal = gs_in[index].normal;
  gs_out.fragment_light_space = gs_in[index].fragment_light_space;
}

vec3 create_normal(vec3 v1, vec3 v2, vec3 v3) {
  vec3 b = v1 - v2;
  vec3 a = v3 - v2;
  return normalize(cross(a, b));
}

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
  // vec3 normal = create_normal(vec3(gl_in[0].gl_Position), vec3(gl_in[1].gl_Position), vec3(gl_in[2].gl_Position));
  vec3 ws_normal = create_normal(gs_in[0].fragment_position, gs_in[1].fragment_position, gs_in[2].fragment_position);

  gl_Position = projection * view * vec4(gs_in[0].fragment_position+ws_normal, 1.0f);//gl_in[0].gl_Position;// + vec4(normal,0.0f);
  vs_out_to_gs_out(0);
  gs_out.fragment_position += ws_normal;
  gs_out.normal = ws_normal;
  EmitVertex();
  gl_Position = projection * view * vec4(gs_in[1].fragment_position+ws_normal, 1.0f);//gl_in[1].gl_Position;// + vec4(normal,0.0f);
  vs_out_to_gs_out(1);
  gs_out.fragment_position += ws_normal;
  gs_out.normal = ws_normal;
  EmitVertex();
  gl_Position = projection * view * vec4(gs_in[2].fragment_position+ws_normal, 1.0f);//gl_in[2].gl_Position;// + vec4(normal,0.0f);
  vs_out_to_gs_out(2);
  gs_out.fragment_position += ws_normal;
  gs_out.normal = ws_normal;
  EmitVertex();
  EndPrimitive();
}
