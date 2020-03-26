#version 450

layout(location=0) in vec3 vertex_position;
layout(location=1) in vec3 vertex_normal;
layout(location=2) in vec2 vertex_texture_coordinate;
layout(location=3) in ivec4 vertex_ids;
layout(location=4) in vec4 vertex_weights;

out VS_OUT {
	vec3 fragment_position;
	vec2 texture_coordinate;
	vec3 normal;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

#define MAX_BONES 10
uniform mat4 armature[MAX_BONES];

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

	vs_out.fragment_position = vec3(model * bone_transform * vec4(vertex_position, 1.0f));
	vs_out.texture_coordinate = vertex_texture_coordinate;
	vs_out.normal = mat3(transpose(inverse(model*bone_transform))) * vertex_normal;
	gl_Position = projection * view * vec4(vs_out.fragment_position, 1.0);
}
