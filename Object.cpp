#include <QDebug>

#include "Object.h"

Object::Object(Material material, glm::vec3 position, glm::vec3 scale) :
  material(material),
  position(position),
  scale(scale)
{}

Object::Object() {
  material = {glm::vec3(0.1f),glm::vec3(1.0f),glm::vec3(1.0f),0.0f};
  position = glm::vec3(0.0f);
  scale = glm::vec3(1.0f);
}

Object::~Object() {}

void Object::initialize_shader(Shader *shader) {
  initializeOpenGLFunctions();
  this->shader = shader;
}

void Object::initialize_buffers() {
  float vertices[] = {
  //Position            Normal             Texture
    // Front
    +0.5f,+0.5f,+0.5f, +0.0f,+0.0f,+1.0f,  1.0f, 1.0f, // right top front
    -0.5f,+0.5f,+0.5f, +0.0f,+0.0f,+1.0f,  0.0f, 1.0f, // left top front
    +0.5f,-0.5f,+0.5f, +0.0f,+0.0f,+1.0f,  1.0f, 0.0f, // right bottom front
    -0.5f,-0.5f,+0.5f, +0.0f,+0.0f,+1.0f,  0.0f, 0.0f, // left bottom front
    // Back
    +0.5f,+0.5f,-0.5f, +0.0f,+0.0f,-1.0f,  1.0f, 1.0f, // right top back
    -0.5f,+0.5f,-0.5f, +0.0f,+0.0f,-1.0f,  0.0f, 1.0f, // left top back
    +0.5f,-0.5f,-0.5f, +0.0f,+0.0f,-1.0f,  1.0f, 0.0f, // right bottom back
    -0.5f,-0.5f,-0.5f, +0.0f,+0.0f,-1.0f,  0.0f, 0.0f, // left bottom back
    // Right
    +0.5f,+0.5f,-0.5f, +1.0f,+0.0f,+0.0f,  1.0f, 1.0f, // right top back
    +0.5f,+0.5f,+0.5f, +1.0f,+0.0f,+0.0f,  0.0f, 1.0f, // right top front
    +0.5f,-0.5f,-0.5f, +1.0f,+0.0f,+0.0f,  1.0f, 0.0f, // right bottom back
    +0.5f,-0.5f,+0.5f, +1.0f,+0.0f,+0.0f,  0.0f, 0.0f, // right bottom front
    // Left
    -0.5f,+0.5f,-0.5f, -1.0f,+0.0f,+0.0f,  1.0f, 1.0f, // left top back
    -0.5f,+0.5f,+0.5f, -1.0f,+0.0f,+0.0f,  0.0f, 1.0f, // left top front
    -0.5f,-0.5f,-0.5f, -1.0f,+0.0f,+0.0f,  1.0f, 0.0f, // left bottom back
    -0.5f,-0.5f,+0.5f, -1.0f,+0.0f,+0.0f,  0.0f, 0.0f, // left bottom front
    // Top
    +0.5f,+0.5f,-0.5f, +0.0f,+1.0f,+0.0f,  1.0f, 1.0f, // right top back
    -0.5f,+0.5f,-0.5f, +0.0f,+1.0f,+0.0f,  0.0f, 1.0f, // left top back
    +0.5f,+0.5f,+0.5f, +0.0f,+1.0f,+0.0f,  1.0f, 0.0f, // right top front
    -0.5f,+0.5f,+0.5f, +0.0f,+1.0f,+0.0f,  0.0f, 0.0f, // left top front
    // Bottom
    +0.5f,-0.5f,-0.5f, +0.0f,-1.0f,+0.0f,  1.0f, 1.0f, // right bottom back
    -0.5f,-0.5f,-0.5f, +0.0f,-1.0f,+0.0f,  0.0f, 1.0f, // left bottom back
    +0.5f,-0.5f,+0.5f, +0.0f,-1.0f,+0.0f,  1.0f, 0.0f, // right bottom front
    -0.5f,-0.5f,+0.5f, +0.0f,-1.0f,+0.0f,  0.0f, 0.0f // left bottom front
  };
  unsigned int indices[] = {
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

  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glGenVertexArrays(1, &VAO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(3*sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(6*sizeof(float)));
  glEnableVertexAttribArray(2);
}

void Object::initialize_buffers(unsigned int vbo, unsigned int ebo) {
  VBO = vbo;
  EBO = ebo;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(3*sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (GLvoid*)(6*sizeof(float)));
  glEnableVertexAttribArray(2);
}

void Object::load_texture(const char *image_path, const char *image_name) {
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  QImage img = QImage(image_path).convertToFormat(QImage::Format_RGB888).mirrored( true, true);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
  glGenerateMipmap(GL_TEXTURE_2D);

  shader->use();
  shader->setInt(image_name, textures.size());
  textures.push_back(texture);
}

void Object::draw() {
  for (int i=0; i<int(textures.size()); i++) {
    glActiveTexture(GL_TEXTURE0+i);
    glBindTexture(GL_TEXTURE_2D, textures[i]);
  }

  shader->setVec3("material.albedo", material.diffuse);
  shader->setVec3("material.ambient", material.ambient);
  shader->setFloat("material.specularity", 1.0f);
  shader->setInt("material.shininess", material.shininess);
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)0);
}

// Getters
glm::mat4 Object::get_model_matrix() {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
  model = glm::scale(model, glm::vec3(scale));
  return model;
}

glm::vec3 Object::get_position() {return position;}
glm::vec3 Object::get_scale() {return scale;}
float Object::get_shininess() {return material.shininess;}
unsigned int Object::get_VBO() {return VBO;}
unsigned int Object::get_EBO() {return EBO;}

// Setters
void Object::set_position(glm::vec3 position) {this->position=position;}
void Object::set_x_position(float x) {position.x=x;}
void Object::set_y_position(float y) {position.y=y;}
void Object::set_z_position(float z) {position.z=z;}
void Object::set_scale(glm::vec3 scale) {this->scale=scale;}
void Object::set_x_scale(float x) {scale.x=x;}
void Object::set_y_scale(float y) {scale.y=y;}
void Object::set_z_scale(float z) {scale.z=z;}
void Object::set_shininess(float shininess) {material.shininess=shininess;}
