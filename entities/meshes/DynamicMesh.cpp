#include "DynamicMesh.h"

void DynamicMesh::initialize_buffers() {
  initializeOpenGLFunctions();

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

  // Position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  // Normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(1);
  // Texture Coordinate
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coordinate));
  glEnableVertexAttribArray(2);
  // Bone IDs
  glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, bone_ids));
  glEnableVertexAttribArray(3);
  // Bone Weights
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bone_weights));
  glEnableVertexAttribArray(4);

  // Unbind vertex array
  glBindVertexArray(0);
}

void DynamicMesh::update_vertex_buffer(bool size_changed) {
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  if (size_changed) {
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
  } else {
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size()*sizeof(Vertex), vertices.data());
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DynamicMesh::update_index_buffer(bool size_changed) {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  if (size_changed) {
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
  } else {
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size()*sizeof(unsigned int), indices.data());
  }
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
