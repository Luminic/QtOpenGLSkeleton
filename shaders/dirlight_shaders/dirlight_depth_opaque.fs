#version 450

#define TRANSPARENCY_OPAQUE 1
#define TYPE_DIRLIGHT 1

layout (depth_greater) out float gl_FragDepth;

void main() {
  gl_FragDepth = gl_FragCoord.z;
  gl_FragDepth += gl_FrontFacing ? 0.002 : 0.0;
}
