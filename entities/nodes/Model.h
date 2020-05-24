#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <vector>
#include <unordered_map>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "../../rendering/Shader.h"
#include "../meshes/Mesh.h"
#include "../meshes/Material.h"
#include "Node.h"
#include "RootNode.h"

class Model : public RootNode {
  Q_OBJECT;

public:
  Model(const char *path);
  Model(const char *path, const char* name);
  virtual ~Model();

protected:
  void load_model(std::string path);
  Node* process_node(aiNode* node, const aiScene* scene, bool root=false);
  Mesh* process_mesh(aiMesh* mesh, const aiScene* scene);
  void load_material_textures(aiMaterial* mat, Material* mesh_material);
  void load_armature(Node* node);
  void load_animations(const aiScene* scene);

  std::unordered_map<std::string, unsigned int> loaded_bones;
  std::unordered_map<std::string, Node*> loaded_nodes;

  std::string directory;

  std::vector<Texture> textures_loaded;
};

#endif
