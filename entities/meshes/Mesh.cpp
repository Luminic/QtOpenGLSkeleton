#include <QDebug>

#include "Mesh.h"

int Mesh::nr_meshes_created = 0;

Mesh::Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices, Material *material) :
  vertices(vertices),
  indices(indices),
  material(material)
{
  init();
  initialize_buffers();
}

Mesh::Mesh() {
  init();
}

void Mesh::init() {
  transparency = OPAQUE;
  name = "mesh #" + std::to_string(nr_meshes_created);
}

Mesh::~Mesh() {
}

void Mesh::draw(Shader* shader, Shader::DrawType draw_type, const glm::mat4& model, int texture_unit) {
  shader->use();
  shader->setMat4("model", model);
  if (draw_type == Shader::DrawType::COLOR) {
    material->draw(shader, texture_unit);
  }
  if (transparency != Transparency::OPAQUE) {
    material->set_opacity_map(shader, texture_unit);
  }

  // Draw Mesh
  simple_draw();
}

void Mesh::simple_draw() {
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Mesh::initialize_cube(float texture_scale) {
  vertices = {
    Vertex({glm::vec3(+0.5f,+0.5f,+0.5f), glm::vec3(+0.0f,+0.0f,+1.0f), glm::vec2(texture_scale, texture_scale), glm::ivec4(0), glm::vec4(0.0f) }), // right top front
    Vertex({glm::vec3(-0.5f,+0.5f,+0.5f), glm::vec3(+0.0f,+0.0f,+1.0f), glm::vec2(0.0f, texture_scale),          glm::ivec4(0), glm::vec4(0.0f) }), // left top front
    Vertex({glm::vec3(+0.5f,-0.5f,+0.5f), glm::vec3(+0.0f,+0.0f,+1.0f), glm::vec2(texture_scale, 0.0f),          glm::ivec4(0), glm::vec4(0.0f) }), // right bottom front
    Vertex({glm::vec3(-0.5f,-0.5f,+0.5f), glm::vec3(+0.0f,+0.0f,+1.0f), glm::vec2(0.0f, 0.0f),                   glm::ivec4(0), glm::vec4(0.0f) }), // left bottom front
    // Back
    Vertex({glm::vec3(+0.5f,+0.5f,-0.5f), glm::vec3(+0.0f,+0.0f,-1.0f), glm::vec2(texture_scale, texture_scale), glm::ivec4(0), glm::vec4(0.0f) }), // right top back
    Vertex({glm::vec3(-0.5f,+0.5f,-0.5f), glm::vec3(+0.0f,+0.0f,-1.0f), glm::vec2(0.0f, texture_scale),          glm::ivec4(0), glm::vec4(0.0f) }), // left top back
    Vertex({glm::vec3(+0.5f,-0.5f,-0.5f), glm::vec3(+0.0f,+0.0f,-1.0f), glm::vec2(texture_scale, 0.0f),          glm::ivec4(0), glm::vec4(0.0f) }), // right bottom back
    Vertex({glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(+0.0f,+0.0f,-1.0f), glm::vec2(0.0f, 0.0f),                   glm::ivec4(0), glm::vec4(0.0f) }), // left bottom back
    // Right
    Vertex({glm::vec3(+0.5f,+0.5f,-0.5f), glm::vec3(+1.0f,+0.0f,+0.0f), glm::vec2(texture_scale, texture_scale), glm::ivec4(0), glm::vec4(0.0f) }), // right top back
    Vertex({glm::vec3(+0.5f,+0.5f,+0.5f), glm::vec3(+1.0f,+0.0f,+0.0f), glm::vec2(0.0f, texture_scale),          glm::ivec4(0), glm::vec4(0.0f) }), // right top front
    Vertex({glm::vec3(+0.5f,-0.5f,-0.5f), glm::vec3(+1.0f,+0.0f,+0.0f), glm::vec2(texture_scale, 0.0f),          glm::ivec4(0), glm::vec4(0.0f) }), // right bottom back
    Vertex({glm::vec3(+0.5f,-0.5f,+0.5f), glm::vec3(+1.0f,+0.0f,+0.0f), glm::vec2(0.0f, 0.0f),                   glm::ivec4(0), glm::vec4(0.0f) }), // right bottom front
    // Left
    Vertex({glm::vec3(-0.5f,+0.5f,-0.5f), glm::vec3(-1.0f,+0.0f,+0.0f), glm::vec2(texture_scale, texture_scale), glm::ivec4(0), glm::vec4(0.0f) }), // left top back
    Vertex({glm::vec3(-0.5f,+0.5f,+0.5f), glm::vec3(-1.0f,+0.0f,+0.0f), glm::vec2(0.0f, texture_scale),          glm::ivec4(0), glm::vec4(0.0f) }), // left top front
    Vertex({glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(-1.0f,+0.0f,+0.0f), glm::vec2(texture_scale, 0.0f),          glm::ivec4(0), glm::vec4(0.0f) }), // left bottom back
    Vertex({glm::vec3(-0.5f,-0.5f,+0.5f), glm::vec3(-1.0f,+0.0f,+0.0f), glm::vec2(0.0f, 0.0f),                   glm::ivec4(0), glm::vec4(0.0f) }), // left bottom front
    // Top
    Vertex({glm::vec3(+0.5f,+0.5f,-0.5f), glm::vec3(+0.0f,+1.0f,+0.0f), glm::vec2(texture_scale, texture_scale), glm::ivec4(0), glm::vec4(0.0f) }), // right top back
    Vertex({glm::vec3(-0.5f,+0.5f,-0.5f), glm::vec3(+0.0f,+1.0f,+0.0f), glm::vec2(0.0f, texture_scale),          glm::ivec4(0), glm::vec4(0.0f) }), // left top back
    Vertex({glm::vec3(+0.5f,+0.5f,+0.5f), glm::vec3(+0.0f,+1.0f,+0.0f), glm::vec2(texture_scale, 0.0f),          glm::ivec4(0), glm::vec4(0.0f) }), // right top front
    Vertex({glm::vec3(-0.5f,+0.5f,+0.5f), glm::vec3(+0.0f,+1.0f,+0.0f), glm::vec2(0.0f, 0.0f),                   glm::ivec4(0), glm::vec4(0.0f) }), // left top front
    // Bottom
    Vertex({glm::vec3(+0.5f,-0.5f,-0.5f), glm::vec3(+0.0f,-1.0f,+0.0f), glm::vec2(texture_scale, texture_scale), glm::ivec4(0), glm::vec4(0.0f) }), // right bottom back
    Vertex({glm::vec3(-0.5f,-0.5f,-0.5f), glm::vec3(+0.0f,-1.0f,+0.0f), glm::vec2(0.0f, texture_scale),          glm::ivec4(0), glm::vec4(0.0f) }), // left bottom back
    Vertex({glm::vec3(+0.5f,-0.5f,+0.5f), glm::vec3(+0.0f,-1.0f,+0.0f), glm::vec2(texture_scale, 0.0f),          glm::ivec4(0), glm::vec4(0.0f) }), // right bottom front
    Vertex({glm::vec3(-0.5f,-0.5f,+0.5f), glm::vec3(+0.0f,-1.0f,+0.0f), glm::vec2(0.0f, 0.0f),                   glm::ivec4(0), glm::vec4(0.0f) }) // left bottom front
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

void Mesh::initialize_plane(bool horizontal, float texture_scale) {
  if (horizontal) {
    vertices = {
      Vertex({glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec2(0.0f,texture_scale),          glm::ivec4(0), glm::vec4(0.0f) }),
      Vertex({glm::vec3( 1.0f, 0.0f, 1.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec2(texture_scale,texture_scale), glm::ivec4(0), glm::vec4(0.0f) }),
      Vertex({glm::vec3(-1.0f, 0.0f,-1.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec2(0.0f,0.0f),                   glm::ivec4(0), glm::vec4(0.0f) }),
      Vertex({glm::vec3( 1.0f, 0.0f,-1.0f), glm::vec3(0.0f,1.0f,0.0f), glm::vec2(texture_scale,0.0f),          glm::ivec4(0), glm::vec4(0.0f) })
    };
    indices = {
      0, 1, 2,
      1, 3, 2
    };
  } else {
    vertices = {
      Vertex({glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f,0.0f,1.0f), glm::vec2(0.0f,texture_scale),          glm::ivec4(0), glm::vec4(0.0f) }),
      Vertex({glm::vec3( 1.0f, 1.0f, 0.0f), glm::vec3(0.0f,0.0f,1.0f), glm::vec2(texture_scale,texture_scale), glm::ivec4(0), glm::vec4(0.0f) }),
      Vertex({glm::vec3(-1.0f,-1.0f, 0.0f), glm::vec3(0.0f,0.0f,1.0f), glm::vec2(0.0f,0.0f),                   glm::ivec4(0), glm::vec4(0.0f) }),
      Vertex({glm::vec3( 1.0f,-1.0f, 0.0f), glm::vec3(0.0f,0.0f,1.0f), glm::vec2(texture_scale,0.0f),          glm::ivec4(0), glm::vec4(0.0f) })
    };
    indices = {
      2, 1, 0,
      2, 3, 1
    };
  }
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
  // Bone IDs
  glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, bone_ids));
  glEnableVertexAttribArray(3);
  // Bone Weights
  glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bone_weights));
  glEnableVertexAttribArray(4);

  // Unbind vertex array
  glBindVertexArray(0);
}
