#version 450

layout(location=0) in vec3 vertex_position;
layout(location=1) in vec3 vertex_normal;
layout(location=2) in vec2 vertex_texture_coordinate;
layout(location=3) in ivec4 vertex_ids;
layout(location=4) in vec4 vertex_weights;

#define MAX_BONES 10

// The size of armature is 64 * MAX_BONES
layout (std140, binding=0) uniform Armature {
	mat4 armature[MAX_BONES];
};

out VS_OUT {
	vec3 fragment_position;
	vec2 texture_coordinate;
	vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

float linear_depth(float depth) {
  float near = 0.1;
  float far  = 100.0;
  float z = 2 * depth - 1;
  return (2.0 * near * far) / (far + near - z * (far - near)) / far;
}

void main() {
	mat4 bone_transform;
	float vertex_weight_total = vertex_weights[0]+vertex_weights[1]+vertex_weights[2]+vertex_weights[3];
	if (vertex_weight_total <= 0.001) {
		bone_transform = mat4(1.0f);
	} else {
		bone_transform = armature[vertex_ids[0]] * vertex_weights[0]/vertex_weight_total;
		bone_transform += armature[vertex_ids[1]] * vertex_weights[1]/vertex_weight_total;
		bone_transform += armature[vertex_ids[2]] * vertex_weights[2]/vertex_weight_total;
		bone_transform += armature[vertex_ids[3]] * vertex_weights[3]/vertex_weight_total;
	}

	vs_out.fragment_position = vec3(bone_transform * model * vec4(vertex_position, 1.0f));
	vs_out.texture_coordinate = vertex_texture_coordinate;
	vs_out.normal = transpose(inverse(mat3(bone_transform*model))) * vertex_normal;
	gl_Position = projection * view * vec4(vs_out.fragment_position, 1.0);
	gl_PointSize = 10.0f/gl_Position.z;
}
