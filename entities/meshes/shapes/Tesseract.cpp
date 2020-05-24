#include "Tesseract.h"

#include <QDebug>

Tesseract::Tesseract() {
  reset_vertices();
  indices_4D = {
    // Outer Cube
    // - Front
    glm::ivec4(1, 0, 2, 3),
    // - Right
    glm::ivec4(0, 4, 6, 2),
    // - Back
    glm::ivec4(4, 5, 7, 6),
    // - Left
    glm::ivec4(5, 1, 3, 7),
    // - Top
    glm::ivec4(5, 4, 0, 1),
    // - Bottom
    glm::ivec4(3, 2, 6, 7),

    // Inner Cube
    // - Front
    glm::ivec4(9, 8, 10, 11),
    // - Right
    glm::ivec4(8, 12, 14, 10),
    // - Back
    glm::ivec4(12, 13, 15, 14),
    // - Left
    glm::ivec4(13, 9, 11, 15),
    // - Top
    glm::ivec4(13, 12, 8, 9),
    // - Bottom
    glm::ivec4(11, 10, 14, 15),

    // Connecting Front Faces of Inner & Outer
    // - Top
    glm::ivec4(1, 0, 8, 9),
    // - Right
    glm::ivec4(0, 2, 10, 8),
    // - Bottom
    glm::ivec4(2, 3, 11, 10),
    // - Left
    glm::ivec4(3, 1, 9, 11),

    // Connecting Back Faces of Inner & Outer
    // - Top
    glm::ivec4(4, 5, 13, 12),
    // - Right
    glm::ivec4(6, 4, 12, 14),
    // - Bottom
    glm::ivec4(7, 6, 14, 15),
    // - Left
    glm::ivec4(5, 7, 15, 13),

    // Connecting Right Faces of Inner & Outer
    // - Top
    glm::ivec4(0, 4, 12, 8),
    // - Bottom
    glm::ivec4(6, 2, 10, 14),

    // Connecting Left Faces of Inner & Outer
    // - Top
    glm::ivec4(5, 1, 9, 13),
    // - Bottom
    glm::ivec4(3, 7, 15, 11)
  };

  for (auto face : indices_4D) {
    // Triangulate
    unsigned int trangulated_indices[6] = {
      (unsigned int) face.x, (unsigned int) face.y, (unsigned int) face.w,
      (unsigned int) face.w, (unsigned int) face.y, (unsigned int) face.z
    };
    indices.insert(indices.end(), std::begin(trangulated_indices), std::end(trangulated_indices));

    Vertex vertices_3D[4] = {
      Vertex {glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f), glm::vec2(0.0f,0.0f), glm::ivec4(0.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,0.0f,0.0f)},
      Vertex {glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f), glm::vec2(0.0f,0.0f), glm::ivec4(0.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,0.0f,0.0f)},
      Vertex {glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f), glm::vec2(0.0f,0.0f), glm::ivec4(0.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,0.0f,0.0f)},
      Vertex {glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f), glm::vec2(0.0f,0.0f), glm::ivec4(0.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,0.0f,0.0f)}
    };
    vertices.insert(vertices.end(), std::begin(vertices_3D), std::end(vertices_3D));
    // TODO: Expand each face to have its own vertices in 3d so proper texture coordinates and normals can be calculated
  }

  initialize_buffers();
}

Tesseract::~Tesseract() {}

void Tesseract::reset_vertices() {
  /**
  * Outer Cube:
  *     5--------4
  *    /|       /|
  *  1---------0 |
  *  |  |      | |
  *  |  7------|-6
  *  | /       |/
  *  3---------2
  * Inner Cube is Outer Cube +8
  */
  vertices_4D = {
    // Outer Cube
    // - Front
    Vertex4D{glm::vec4(+0.5f,+0.5f,+0.5f,-0.5)}, // right top front
    Vertex4D{glm::vec4(-0.5f,+0.5f,+0.5f,-0.5)}, // left top front
    Vertex4D{glm::vec4(+0.5f,-0.5f,+0.5f,-0.5)}, // right bottom front
    Vertex4D{glm::vec4(-0.5f,-0.5f,+0.5f,-0.5)}, // left bottom front
    // - Back
    Vertex4D{glm::vec4(+0.5f,+0.5f,-0.5f,-0.5)}, // right top back
    Vertex4D{glm::vec4(-0.5f,+0.5f,-0.5f,-0.5)}, // left top back
    Vertex4D{glm::vec4(+0.5f,-0.5f,-0.5f,-0.5)}, // right bottom back
    Vertex4D{glm::vec4(-0.5f,-0.5f,-0.5f,-0.5)}, // left bottom back
    // Inner Cube
    // Front
    Vertex4D{glm::vec4(+0.5f,+0.5f,+0.5f,+0.5)}, // right top front
    Vertex4D{glm::vec4(-0.5f,+0.5f,+0.5f,+0.5)}, // left top front
    Vertex4D{glm::vec4(+0.5f,-0.5f,+0.5f,+0.5)}, // right bottom front
    Vertex4D{glm::vec4(-0.5f,-0.5f,+0.5f,+0.5)}, // left bottom front
    // Back
    Vertex4D{glm::vec4(+0.5f,+0.5f,-0.5f,+0.5)}, // right top back
    Vertex4D{glm::vec4(-0.5f,+0.5f,-0.5f,+0.5)}, // left top back
    Vertex4D{glm::vec4(+0.5f,-0.5f,-0.5f,+0.5)}, // right bottom back
    Vertex4D{glm::vec4(-0.5f,-0.5f,-0.5f,+0.5)}  // left bottom back
  };
}

void Tesseract::rotate(float angle, rotation_4D::RotationPlane rotation_plane) {
  for (unsigned int i=0; i<vertices_4D.size(); i++) {
    vertices_4D[i].position = rotation_4D::on(angle, rotation_plane) * vertices_4D[i].position;
  }
}

void Tesseract::project_to_3d() {
  // Near plane will be at (0,0,0,-2)
  for (unsigned int i=0; i<vertices_4D.size(); i++) {
    vertices[i].position = glm::vec3(vertices_4D[i].position)*(1.0f/(vertices_4D[i].position.w+2.0f));
  }
  update_vertex_buffer(false);
}

void Tesseract::draw(Shader* shader, Shader::DrawType draw_type, const glm::mat4& model, int texture_unit) {
  glDepthMask(GL_FALSE);
  Mesh::draw(shader, draw_type, model, texture_unit);
  glDepthMask(GL_TRUE);
}
