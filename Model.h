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
#include "Node.h"
#include "Material.h"
#include "Settings.h"

class Model : public Node {
  Q_OBJECT

public:
  Model(const char *path);
  ~Model();

protected:
  void load_model(std::string path);
  Node * process_node(aiNode *node, const aiScene *scene);
  Mesh * process_mesh(aiMesh *mesh, const aiScene *scene);

  void load_material_textures(aiMaterial *mat, Material *mesh_material);

  //std::vector<Mesh*> meshes;
  std::string directory;

  std::vector<Texture> textures_loaded;
};

#endif
