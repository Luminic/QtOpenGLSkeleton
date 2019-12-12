#include <QFile>
#include <QDebug>

#include "Shader.h"

std::string textContent(QString path) {
    QFile file(path);
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream in(&file);
    return in.readAll().toStdString();
}

Shader::Shader() {}

Shader::~Shader() {}

void Shader::loadShaders(const char* vertex_path, const char* fragment_path) {
  initializeOpenGLFunctions();
  unsigned int vert_shader, frag_shader;
  int success;
  char infoLog[512];

  // Load vertex shader
  std::string vertex_shader_str = textContent(vertex_path);
  const char *vertex_shader_code = vertex_shader_str.data();
  // Compile vertex shader
  vert_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_shader, 1, &vertex_shader_code, NULL);
  glCompileShader(vert_shader);
  // Check for errors in the compilation of the vertex shader
  glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(vert_shader, 512, NULL, infoLog);
    qDebug() << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog;
  }

  // Load fragment shader
  std::string fragment_shader_str = textContent(fragment_path);
  const char *fragment_shader_code = fragment_shader_str.data();
  // Compile fragment shader
  frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_shader, 1, &fragment_shader_code, NULL);
  glCompileShader(frag_shader);
  // Check for errors in the compilation of the fragment shader
  glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(frag_shader, 512, NULL, infoLog);
    qDebug() << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog;
  }
  // Create the shader program
  ID = glCreateProgram();
  glAttachShader(ID, frag_shader);
  glAttachShader(ID, vert_shader);
  glLinkProgram(ID);
  // Check for errors
  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if(!success) {
      glGetProgramInfoLog(ID, 512, NULL, infoLog);
      qDebug() << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog;
  }

  // Delete the shaders (they are already linked so they are no longer needed)
  glDeleteShader(vert_shader);
  glDeleteShader(frag_shader);
}

void Shader::use() {
  glUseProgram(ID);
}

void Shader::setInt(const char *name, int value) {
  unsigned int loc = glGetUniformLocation(ID, name);
  glUniform1i(loc, value);
}

void Shader::setFloat(const char *name, float value) {
  unsigned int loc = glGetUniformLocation(ID, name);
  glUniform1f(loc, value);
}

void Shader::setVec3(const char *name, const glm::vec3 &vec) {
  unsigned int loc = glGetUniformLocation(ID, name);
  glUniform3fv(loc, 1, &vec[0]);
}

void Shader::setMat4(const char *name, const glm::mat4 &mat) {
  unsigned int loc = glGetUniformLocation(ID, name);
  glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}
