#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"
#include "Object.h"

class Model : public Object {
  Q_OBJECT

public:
  Model(const char *path);
  ~Model();

  void draw(Shader *shader, glm::mat4 &model);

protected:
  void load_model(std::string path);
  void process_node(aiNode *node, const aiScene *scene, const glm::mat4 &transformation);
  Mesh * process_mesh(aiMesh *mesh, const aiScene *scene, const glm::mat4 &transformation);
  std::vector<Texture> load_material_textures(aiMaterial *mat, aiTextureType type, std::string type_name);
  unsigned int load_texture(const char *path, std::string directory);

  std::vector<Mesh*> meshes;
  std::string directory;

  std::vector<Texture> textures_loaded;
};

#endif
