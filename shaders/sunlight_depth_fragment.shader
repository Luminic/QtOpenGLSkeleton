#version 450

layout (depth_greater) out float gl_FragDepth;

//out vec4 fragment_color;

void main() {
  gl_FragDepth = gl_FragCoord.z;
  gl_FragDepth += gl_FrontFacing ? 0.005 : 0.0;
  //fragment_color = vec4(gl_FragCoord.zzz, 0.0f);
}
