#include <QFile>
#include <QDebug>

#include "Shader.h"
#include "../entities/meshes/Material.h"

unsigned int Shader::placeholder_texture = 0;
std::unordered_map<std::string, unsigned int> Shader::uniform_block_buffers;

QString textContent(QString path, std::shared_ptr<std::vector<QString>> already_included_names = std::make_shared<std::vector<QString>>()) {
  QFile file(path);
  file.open(QFile::ReadOnly | QFile::Text);
  QTextStream in(&file);
  unsigned int current_line_number = 0;
  QString current_content = "";
  while (!in.atEnd()) {
    current_line_number++;
    // readLine() strips \n so we have to re-add the \n
    QString current_line = in.readLine()+'\n';

    // Is a mypreprocessor directive
    if (current_line.contains("#mypreprocessor")) {
      // Is an include directive
      if (current_line.contains("include")) {
        QString include_file_name = current_line.section("\"", 1, 1);
        QString path_without_file_name = path.section('/',0,-2);
        if (path_without_file_name != "") {
          path_without_file_name += '/';
        }
        QString include_file_path = path_without_file_name+include_file_name;

        // Check if the file has already been included
        bool already_included = false;
        for (auto name : *already_included_names) {
          if (include_file_name == name) {
            already_included = true;
            current_line = "\n";
          }
        }
        if (!already_included) {
          // Add the included file to the shader
          current_line = textContent(include_file_path, already_included_names);
          already_included_names->push_back(include_file_name);
        }
      } else {
        qWarning() << "Unknown include directive" << current_line << "in" << path << "line" << current_line_number;
        current_line = "\n";
      }
    }

    current_content.append(current_line);
  }
  return current_content;
  // return in.readAll().toStdString();
}

Shader::Shader() {}

Shader::~Shader() {}

void Shader::loadShaders(const char* vertex_path, const char* fragment_path, const char *geometry_path) {
  initializeOpenGLFunctions();
  unsigned int vert_shader, frag_shader;
  int success;
  char infoLog[512];

  // Create the shader program
  ID = glCreateProgram();

  // Load vertex shader
  std::string vertex_shader_str = textContent(vertex_path).toStdString();
  const char* vertex_shader_code = vertex_shader_str.data();
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
  glAttachShader(ID, vert_shader);
  glDeleteShader(vert_shader);

  // Load fragment shader
  std::string fragment_shader_str = textContent(fragment_path).toStdString();
  const char* fragment_shader_code = fragment_shader_str.data();
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
  glAttachShader(ID, frag_shader);
  glDeleteShader(frag_shader);

  if (geometry_path[0] != '\0') {
    // Load geometry shader
    std::string geometry_shader_str = textContent(geometry_path).toStdString();
    const char* geometry_shader_code = geometry_shader_str.data();
    // Compile geometry shader
    unsigned int geom_shader = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geom_shader, 1, &geometry_shader_code, NULL);
    glCompileShader(geom_shader);
    // Check for errors
    glGetShaderiv(geom_shader, GL_COMPILE_STATUS, &success);
    if(!success) {
      glGetShaderInfoLog(geom_shader, 512, NULL, infoLog);
      qDebug() << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog;
    }
    glAttachShader(ID, geom_shader);
    glDeleteShader(geom_shader);
  }

  glLinkProgram(ID);
  // Check for errors
  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if(!success) {
    glGetProgramInfoLog(ID, 512, NULL, infoLog);
    qDebug() << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog;
  }
}

void Shader::initialize_placeholder_textures(Image_Type texture_types) {
  if (Shader::placeholder_texture == 0) {
    Shader::placeholder_texture = Material::static_load_texture("assets/textures/placeholder_texture.png", Image_Type::ALBEDO_MAP).id;
  }

  use();
  if (texture_types & Image_Type::ALBEDO_MAP) {
    setInt("material.albedo_map", 0);
  }
  if (texture_types & Image_Type::AMBIENT_OCCLUSION_MAP) {
    setInt("material.ambient_occlusion_map", 0);
  }
  if (texture_types & Image_Type::ROUGHNESS_MAP) {
    setInt("material.roughness_map", 0);
  }
  if (texture_types & Image_Type::METALNESS_MAP) {
    setInt("material.metalness_map", 0);
  }
  if (texture_types & Image_Type::OPACITY_MAP) {
    setInt("material.opacity_map", 0);
  }
}

void Shader::initialize_placeholder_2D_textures(std::vector<const char*> texture_names) {
  if (Shader::placeholder_texture == 0) {
    Shader::placeholder_texture = Material::static_load_texture("textures/placeholder_texture.png", Image_Type::ALBEDO_MAP).id;
  }

  use();
  for (auto name : texture_names) {
    setInt(name, 0);
  }
}

bool Shader::validate_program() {
  int success;
  char infoLog[512];
  // Validate the program
  glValidateProgram(ID);
  glGetProgramiv(ID, GL_VALIDATE_STATUS, &success);
  if(!success) {
    glGetProgramInfoLog(ID, 512, NULL, infoLog);
    qDebug() << "WARNING::SHADER::PROGRAM::VALIDATION_FAILED\n" << infoLog;
  }
  return success;
}

void Shader::use() {
  glUseProgram(ID);
}

void Shader::setBool(const char *name, bool value) {
  unsigned int loc = glGetUniformLocation(ID, name);
  glUniform1i(loc, (int)value);
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
