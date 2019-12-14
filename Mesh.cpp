#include <QDebug>

#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, std::vector<Texture> &textures, Material material, glm::mat4 transformation) : Object(material),
  vertices(vertices),
  indices(indices),
  textures(textures),
  transformation(transformation)
{
  setup_mesh();
}

Mesh::~Mesh() {}

void Mesh::draw(Shader *shader, glm::mat4 model) {
  shader->setMat4("model", model*transformation);

  // Set Textures
  int number_diffuse_textures = 0;
  bool use_specular_map = false;
  shader->use();

  for (unsigned int i=0; i<textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0+i);

    std::string name = textures[i].type;
    std::string number;
    if (name == "albedo_map") {
      number_diffuse_textures++;
      number = std::to_string(number_diffuse_textures);
      shader->setInt(("material."+name+"["+number+"]").c_str(), i);
    } else if (name == "specular_map") {
      use_specular_map = true;
      shader->setInt(("material."+name).c_str(), i);
    }

    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }

  shader->setFloat("material.metalness", 0.9f);
  shader->setInt("material.number_albedo_maps", number_diffuse_textures);
  shader->setBool("material.use_specular_map", use_specular_map);
  shader->setBool("material.use_ambient_occlusion_map", false);

  shader->setVec3("material.ambient", material.ambient);
  shader->setVec3("material.albedo", material.diffuse);
  shader->setFloat("material.specularity", 1.0f);
  shader->setFloat("material.shininess", material.shininess);
  // Draw Mesh
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);
}

void Mesh::setup_mesh() {
  initializeOpenGLFunctions();

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

  // Position
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
  // Normal
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(1);
  // Texture Coordinate
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coordinate));
  glEnableVertexAttribArray(2);
}
