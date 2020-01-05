#version 450

out vec4 frag_color;

in vec2 texture_coordinate;

uniform sampler2D texture_diffuse1;

void main() {
	frag_color = texture(texture_diffuse1, texture_coordinate);
}
