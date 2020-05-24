#ifndef FOUR_DIMENSIONAL_ROTATIONS_H
#define FOUR_DIMENSIONAL_ROTATIONS_H

#include <glm/glm.hpp>

namespace rotation_4D {
  enum RotationPlane {
    XY,
    XZ,
    XW,
    YZ,
    YW,
    ZW
  };

  using namespace glm; // I'm not writing glm:: a million times. Sue me.

  // a is the angle in radians
  inline mat4 xy(float a) {
    return mat4(
      cos(a),-sin(a), 0.0f,   0.0f,
      sin(a), cos(a), 0.0f,   0.0f,
      0.0f,   0.0f,   1.0f,   0.0f,
      0.0f,   0.0f,   0.0f,   1.0f
    );
  }

  // a is the angle in radians
  inline mat4 xz(float a) {
    return mat4(
      cos(a), 0.0f,  -sin(a), 0.0f,
      0.0f,   1.0f,   0.0f,   0.0f,
      sin(a), 0.0f,   cos(a), 0.0f,
      0.0f,   0.0f,   0.0f,   1.0f
    );
  }

  // a is the angle in radians
  inline mat4 xw(float a) {
    return mat4(
      cos(a), 0.0f,   0.0f,  -sin(a),
      0.0f,   1.0f,   0.0f,   0.0f,
      0.0f,   0.0f,   1.0f,   0.0f,
      sin(a), 0.0f,   0.0f,   cos(a)
    );
  }

  // a is the angle in radians
  inline mat4 yz(float a) {
    return mat4(
      1.0f,   0.0f,   0.0f,   0.0f,
      0.0f,   cos(a),-sin(a), 0.0f,
      0.0f,   sin(a), cos(a), 0.0f,
      0.0f,   0.0f,   0.0f,   1.0f
    );
  }

  // a is the angle in radians
  inline mat4 yw(float a) {
    return mat4(
      1.0f,   0.0f,   0.0f,   0.0f,
      0.0f,   cos(a), 0.0f,  -sin(a),
      0.0f,   0.0f,   1.0f,   0.0f,
      0.0f,   sin(a), 0.0f,   cos(a)
    );
  }

  // a is the angle in radians
  inline mat4 zw(float a) {
    return mat4(
      1.0f,   0.0f,   0.0f,   0.0f,
      0.0f,   1.0f,   0.0f,   0.0f,
      0.0f,   0.0f,   cos(a),-sin(a),
      0.0f,   0.0f,   sin(a), cos(a)
    );
  }

  inline mat4 on(float a, RotationPlane rotation_plane) {
    switch (rotation_plane) {
      case RotationPlane::XY:
        return xy(a);
        break;
      case RotationPlane::XZ:
        return xz(a);
        break;
      case RotationPlane::XW:
        return xw(a);
        break;
      case RotationPlane::YZ:
        return yz(a);
        break;
      case RotationPlane::YW:
        return yw(a);
        break;
      default:
        return zw(a);
        break;
    };
  }
}

#endif
