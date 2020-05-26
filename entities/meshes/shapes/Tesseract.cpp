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

  unsigned int nr_vertices = 0;
  for (unsigned int i=0; i<indices_4D.size(); i++) {
    // Each face will have its own corner vertices shared with no other faces
    // This way, each face can have its own normal
    Vertex vertices_3D[4] = {
      Vertex {glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec2(0.0f,1.0f), glm::ivec4(0.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,0.0f,0.0f)},
      Vertex {glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec2(1.0f,1.0f), glm::ivec4(0.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,0.0f,0.0f)},
      Vertex {glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec2(1.0f,0.0f), glm::ivec4(0.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,0.0f,0.0f)},
      Vertex {glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec2(0.0f,0.0f), glm::ivec4(0.0f,0.0f,0.0f,0.0f), glm::vec4(0.0f,0.0f,0.0f,0.0f)}
    };
    vertices.insert(vertices.end(), std::begin(vertices_3D), std::end(vertices_3D));

    // Expand the indices
    // indices4D[i] should be the same face as (indices[i*6]..indices[i*6+5])
    unsigned int trangulated_indices[6] = {
      0+nr_vertices, 1+nr_vertices, 3+nr_vertices,
      3+nr_vertices, 1+nr_vertices, 2+nr_vertices
    };
    indices.insert(indices.end(), std::begin(trangulated_indices), std::end(trangulated_indices));
    nr_vertices += 4;
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
  for (unsigned int i=0; i<indices_4D.size(); i++) {
    // Face indices are in the form 0,1,3, 3,1,2
    // 0----1
    // | ,' |
    // 3----2
    // So the unique vertices are indices[i*6], indices[i*6+1], indices[i*6+5], indices[i*6+2]
    // Note the ordering: indices must be ordered this way to match with the ordering of indices4D
    // Camera will be at <0,0,0,-2> facing towards +w to simplify the projection calculation
    vertices[indices[i*6 +0]].position = glm::vec3(vertices_4D[indices_4D[i].x].position) / (vertices_4D[indices_4D[i].x].position.w + 2.0f);
    vertices[indices[i*6 +1]].position = glm::vec3(vertices_4D[indices_4D[i].y].position) / (vertices_4D[indices_4D[i].y].position.w + 2.0f);
    vertices[indices[i*6 +5]].position = glm::vec3(vertices_4D[indices_4D[i].z].position) / (vertices_4D[indices_4D[i].z].position.w + 2.0f);
    vertices[indices[i*6 +2]].position = glm::vec3(vertices_4D[indices_4D[i].w].position) / (vertices_4D[indices_4D[i].w].position.w + 2.0f);

    // Calculate normals
    glm::vec3 a = vertices[indices[i*6 +1]].position - vertices[indices[i*6 +0]].position;
    glm::vec3 b = vertices[indices[i*6 +5]].position - vertices[indices[i*6 +0]].position;
    glm::vec3 norm = glm::cross(a,b);

    vertices[indices[i*6 +0]].normal = norm;
    vertices[indices[i*6 +1]].normal = norm;
    vertices[indices[i*6 +5]].normal = norm;
    vertices[indices[i*6 +2]].normal = norm;
  }
  update_vertex_buffer(false);
}

void Tesseract::draw(Shader* shader, Shader::DrawType draw_type, const glm::mat4& model, int texture_unit) {
  glDepthMask(GL_FALSE);
  GLint src;
  GLint dst;
  glGetIntegerv(GL_BLEND_SRC, &src);
  glGetIntegerv(GL_BLEND_DST, &dst);
  if (draw_type == Shader::DrawType::COLOR) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }
  Mesh::draw(shader, draw_type, model, texture_unit);
  if (draw_type == Shader::DrawType::COLOR) {
    glBlendFunc(src, dst);
  }
  glDepthMask(GL_TRUE);
}
