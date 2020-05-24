#ifndef DYNAMIC_MESH_H
#define DYNAMIC_MESH_H

#include "Mesh.h"

class DynamicMesh : public Mesh {
  Q_OBJECT;

public:
  virtual void initialize_buffers() override;
  // If in doubt, say that the size has size_changed
  // Saying the size hasn't changed when it has is going to cause a world of hurt
  void update_vertex_buffer(bool size_changed);
  void update_index_buffer(bool size_changed);

  std::vector<Vertex>* get_vertices() {
    return &vertices;
  }
  std::vector<unsigned int>* get_indices() {
    return &indices;
  }
};

#endif
