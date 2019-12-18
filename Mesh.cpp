#include <QDebug>

#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, Material *material) :
  vertices(vertices),
  indices(indices),
  material(material)
{
  initialize_buffers();
}

Mesh::Mesh() :
material(nullptr) // Declare as null bc compiler doesn't
{}

Mesh::~Mesh() {
  //if (material != nullptr)
    //delete material;
}

void Mesh::draw(Shader *shader) {
  material->set_materials(shader);

  // Draw Mesh
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);
}

void Mesh::initialize_cube() {
  vertices = {
    Vertex({glm::vec3(+0.5f,+0.5f,+0.5f), glm::vec3(+0.0f,+0.0f,+1.0f), glm::vec2(1.0f, 1.0f)}), // right top front
    Vertex({glm::vec3(-0.5f,+0.5f,+0.5f), glm::vec3(+0.0f,+0.0f,+1.0f), glm::vec2(0.0f, 1.0f)}), // left top front
    Vertex({glm::vec3(+0.5f,-0.5f,+0.5f), glm::vec3(+0.0f,+0.0f,+1.0f), glm::vec2(1.0f, 0.0f)}), // right bottom front
    Vertex({glm::vec3(-0.5f,-0.5f,+0.5f), glm::vec3(+0.0f,+0.0f,+1.0f), glm::vec2(0.0f, 0.0f)}), // left bottom front
    // Back
    Vertex({glm::vec3(+0.5f,+0.5f,-0.5f), glm::vec3(+0.0f,+0.0f,-1.0f), glm::vec2(1.0f, 1.0f)}), // right top back
    Vertex({glm::vec3(-0.5f,+0.5f,-0.5f), glm::vec3(+0.0f,+0.0f,-1.0f), glm::vec2(0.0f, 1.0f)}), // left top back
    Vertex({glm::vec3(+0.5f,-0.5f,-0.5f), glm::vec3(+0.0f,+0.0f,-1.0f), glm::vec2(1.0f, 0.0f)}), // right bottom back
    Vertex({glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(+0.0f,+0.0f,-1.0f), glm::vec2(0.0f, 0.0f)}), // left bottom back
    // Right
    Vertex({glm::vec3(+0.5f,+0.5f,-0.5f), glm::vec3(+1.0f,+0.0f,+0.0f), glm::vec2(1.0f, 1.0f)}), // right top back
    Vertex({glm::vec3(+0.5f,+0.5f,+0.5f), glm::vec3(+1.0f,+0.0f,+0.0f), glm::vec2(0.0f, 1.0f)}), // right top front
    Vertex({glm::vec3(+0.5f,-0.5f,-0.5f), glm::vec3(+1.0f,+0.0f,+0.0f), glm::vec2(1.0f, 0.0f)}), // right bottom back
    Vertex({glm::vec3(+0.5f,-0.5f,+0.5f), glm::vec3(+1.0f,+0.0f,+0.0f), glm::vec2(0.0f, 0.0f)}), // right bottom front
    // Left
    Vertex({glm::vec3(-0.5f,+0.5f,-0.5f), glm::vec3(-1.0f,+0.0f,+0.0f), glm::vec2(1.0f, 1.0f)}), // left top back
    Vertex({glm::vec3(-0.5f,+0.5f,+0.5f), glm::vec3(-1.0f,+0.0f,+0.0f), glm::vec2(0.0f, 1.0f)}), // left top front
    Vertex({glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(-1.0f,+0.0f,+0.0f), glm::vec2(1.0f, 0.0f)}), // left bottom back
    Vertex({glm::vec3(-0.5f,-0.5f,+0.5f), glm::vec3(-1.0f,+0.0f,+0.0f), glm::vec2(0.0f, 0.0f)}), // left bottom front
    // Top
    Vertex({glm::vec3(+0.5f,+0.5f,-0.5f), glm::vec3(+0.0f,+1.0f,+0.0f), glm::vec2(1.0f, 1.0f)}), // right top back
    Vertex({glm::vec3(-0.5f,+0.5f,-0.5f), glm::vec3(+0.0f,+1.0f,+0.0f), glm::vec2(0.0f, 1.0f)}), // left top back
    Vertex({glm::vec3(+0.5f,+0.5f,+0.5f), glm::vec3(+0.0f,+1.0f,+0.0f), glm::vec2(1.0f, 0.0f)}), // right top front
    Vertex({glm::vec3(-0.5f,+0.5f,+0.5f), glm::vec3(+0.0f,+1.0f,+0.0f), glm::vec2(0.0f, 0.0f)}), // left top front
    // Bottom
    Vertex({glm::vec3(+0.5f,-0.5f,-0.5f), glm::vec3(+0.0f,-1.0f,+0.0f), glm::vec2(1.0f, 1.0f)}), // right bottom back
    Vertex({glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(+0.0f,-1.0f,+0.0f), glm::vec2(0.0f, 1.0f)}), // left bottom back
    Vertex({glm::vec3(+0.5f,-0.5f,+0.5f), glm::vec3(+0.0f,-1.0f,+0.0f), glm::vec2(1.0f, 0.0f)}), // right bottom front
    Vertex({glm::vec3(-0.5f,-0.5f,+0.5f), glm::vec3(+0.0f,-1.0f,+0.0f), glm::vec2(0.0f, 0.0f)}) // left bottom front
  };
  indices = {
    // Front
    2, 0, 1,
    3, 2, 1,
    // Back
    4, 6, 5,
    6, 7, 5,
    // Right
    10, 8, 9,
    11, 10, 9,
    // Left
    12, 14, 13,
    14, 15, 13,
    // Top
    18, 16, 17,
    19, 18, 17,
    // Bottom
    20, 22, 21,
    22, 23, 21,
  };
  initialize_buffers();
}

void Mesh::initialize_buffers() {
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

  // Unbind vertex array
  glBindVertexArray(0);
}