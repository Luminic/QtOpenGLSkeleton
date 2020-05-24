#ifndef TESSERACT_H
#define TESSERACT_H

#include <vector>
#include <glm/glm.hpp>

#include "../DynamicMesh.h"
#include "rotations_4d.cpp"

struct Vertex4D {
  glm::vec4 position;
  // Normals and TexCoords can be calculated when converting to normal vertices
  // I'm not dealing with 4d armatures so all weights will be 0
};

class Tesseract : public DynamicMesh {
  Q_OBJECT;

public:
  Tesseract();
  virtual ~Tesseract();

  void reset_vertices();
  // 3D vertices will not be updated; project_to_3d should be called before rendering
  // angle is in radians
  void rotate(float angle, rotation_4D::RotationPlane rotation_plane);
  void project_to_3d();

  virtual void draw(Shader* shader, Shader::DrawType draw_type, const glm::mat4& model, int texture_unit) override;

  float xy_angle;
  float xz_angle;
  float xw_angle;
  float yz_angle;
  float yw_angle;
  float zw_angle;

protected:
  std::vector<glm::ivec4> indices_4D;
  std::vector<Vertex4D> vertices_4D;
};

#endif
