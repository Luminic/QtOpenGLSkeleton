#include <QDebug>

#include "Material.h"
#include "Scene.h"

Material::Material() :
  albedo(glm::vec3(1.0f)),
  ambient(glm::vec3(0.2f)),
  roughness(1.0f),
  metalness(0.0f)
{
  initializeOpenGLFunctions();
}

Material::~Material() {
}

void Material::set_materials(Shader *shader, int material_index_offset) {
  shader->use();

  int number_albedo_maps = 0;
  int number_ambient_occlusion_maps = 0;
  int number_roughness_maps = 0;
  int number_metalness_maps = 0;

  for (unsigned int i=0; i<textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0+i+material_index_offset);
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
    switch (textures[i].type) {
      case ALBEDO_MAP:
        shader->setInt(("material.albedo_map["+std::to_string(number_albedo_maps)+"]").c_str(), i+material_index_offset);
        number_albedo_maps++;
        break;
      case AMBIENT_OCCLUSION_MAP: // AO maps are non-functional at the moment
        number_ambient_occlusion_maps++;
        break;
      case ROUGHNESS_MAP:
        shader->setInt("material.roughness_map", i+material_index_offset);
        number_roughness_maps++;
        break;
      case METALNESS_MAP:
        shader->setInt("material.metalness_map", i+material_index_offset);
        number_metalness_maps++;
        break;
      default:
        break;
    }
  }

  shader->setInt("material.number_albedo_maps", number_albedo_maps);
  shader->setBool("material.use_ambient_occlusion_map", (number_ambient_occlusion_maps>=1));
  shader->setBool("material.use_roughness_map", (number_roughness_maps>=1));
  shader->setBool("material.use_metalness_map", (number_metalness_maps>=1));

  shader->setVec3("material.albedo", albedo);
  shader->setVec3("material.ambient", ambient);
  shader->setFloat("material.roughness", roughness);
  shader->setFloat("material.metalness", metalness);
}

Texture Material::load_texture(const char *path, Image_Type type, bool add_to_material) {
  Texture texture = Scene::is_texture_loaded(path);
  texture.type = type;

  if (texture.id == 0) {
    texture.path = path;

    glGenTextures(1, &texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    QImage img = QImage(path).convertToFormat(QImage::Format_RGB888);
    if (img.isNull()) qDebug() << "ERROR: COULD NOT LOAD IMAGE:" << path;
    if (type == ALBEDO_MAP) { // Convert gamma (SRGB) space into linear (RGB) space
      glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
    } else { // Non-albedo maps should already be in linear space
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
    }
    glGenerateMipmap(GL_TEXTURE_2D);

    Scene::loaded_textures.push_back(texture);
  }

  if (add_to_material)
    textures.push_back(texture);

  return texture;
}

Texture Material::load_cubemap(std::vector<std::string> faces, bool add_to_material) {
  Texture texture;
  texture.path = faces[0];
  texture.type = CUBE_MAP;

  glGenTextures(1, &texture.id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture.id);

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  for (unsigned int i=0; i<faces.size(); i++) {
    QImage img = QImage(faces[i].c_str()).convertToFormat(QImage::Format_RGB888);
    if (img.isNull()) qDebug() << "Could not load cubemap texture:" << faces[i].c_str();
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_SRGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
  }

  if (add_to_material)
    textures.push_back(texture);

  return texture;
}

inline bool compare_floats(float a, float b, float error=0.001) {
  return (glm::abs(a-b) <= error);
}

inline bool compare_vec3(glm::vec3 a, glm::vec3 b, float error=0.001) {
  return compare_floats(a.x, b.x, error) &&
         compare_floats(a.y, b.y, error) &&
         compare_floats(a.z, b.z, error);
}

bool Material::operator==(const Material& other_material) {
  // Check if values match
  if (compare_vec3(albedo, other_material.albedo) &&
    compare_vec3(ambient, other_material.ambient) &&
    compare_floats(roughness, other_material.roughness) &&
    compare_floats(metalness, other_material.metalness)) {
    // Check if there are the same # of textures
    if (textures.size() == other_material.textures.size()) {
      // Check if all the textures match
      for (unsigned int i=0; i<textures.size(); i++) {
        bool other_material_has_texture_i = false;
        for (unsigned int j=0; j<textures.size(); j++) {
          if (textures[i].path == other_material.textures[j].path &&
          textures[i].type == other_material.textures[j].type) {
            other_material_has_texture_i = true;
            break;
          }
        }
        if (other_material_has_texture_i == false) return false;
      }
      return true;
    }
  }
  return false;
}
