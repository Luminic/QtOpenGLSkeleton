#include <QImage>
#include <QDebug>

#include "Model.h"


glm::mat4 aiMat_to_glmMat(const aiMatrix4x4* from) {
  glm::mat4 to(
    {{from->a1,from->b1,from->c1,from->d1}
    ,{from->a2,from->b2,from->c2,from->d2}
    ,{from->a3,from->b3,from->c3,from->d3}
    ,{from->a4,from->b4,from->c4,from->d4}}
  );
  return to;
}


Model::Model(const char *path) {
  initializeOpenGLFunctions();
  load_model(path);
}

Model::~Model() {
  for (auto m : meshes)
    delete m;
}

void Model::load_model(std::string path) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  // Check if the model loaded correctly
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    qDebug() << "ERROR::ASSIMP::" << importer.GetErrorString();
    return;
  }

  directory = path.substr(0, path.find_last_of('/'));
  process_node(scene->mRootNode, scene, glm::mat4(1.0f));
}

void Model::draw(Shader *shader, glm::mat4 &model) {
  for (unsigned int i=0; i<meshes.size(); i++) {
    meshes[i]->draw(shader, model);
  }
}

void Model::process_node(aiNode *node, const aiScene *scene, const glm::mat4 &transformation) {
  glm::mat4 transform = transformation*aiMat_to_glmMat(&node->mTransformation);
  // Process the node's mesh (might be none)
  for (unsigned int i=0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(process_mesh(mesh, scene, transform));
  }

  // Process the node's children (might be none)
  for (unsigned int i=0; i < node->mNumChildren; i++) {
    process_node(node->mChildren[i], scene, transform);
  }
}

Mesh * Model::process_mesh(aiMesh *mesh, const aiScene *scene, const glm::mat4 &transformation) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // Load vertices
  for (unsigned int i=0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    glm::vec3 vector; // Temporary vectors to convert between mesh internal format and Vertex

    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.position = vector;

    vector.x = mesh->mNormals[i].x;
    vector.y = mesh->mNormals[i].y;
    vector.z = mesh->mNormals[i].z;
    vertex.normal = vector;

    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec; // Another Temporary vector
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.texture_coordinate = vec;
    } else {
      vertex.texture_coordinate = glm::vec2(0.0f,0.0f);
    }

    vertices.push_back(vertex);
  }
  // Load indices
  for (unsigned int i=0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j=0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }
  // Load material
  Material *mesh_colors = new Material();
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  // Load textures
  std::vector<Texture> diffuse_maps = load_material_textures(material, aiTextureType_DIFFUSE, "albedo_map");
  textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());
  mesh_colors->albedo_maps = diffuse_maps;
  std::vector<Texture> specular_maps = load_material_textures(material, aiTextureType_SPECULAR, "specular_map");
  textures.insert(textures.end(), specular_maps.begin(), specular_maps.end());
  if (specular_maps.size() > 0) {
    mesh_colors->specular_map = specular_maps.front();
  } else {
    mesh_colors->specular_map = Texture({0,"",""});
  }
  // Load colors
  aiColor3D color(0.0f,0.0f,0.0f);
  float shininess;
  material->Get(AI_MATKEY_COLOR_AMBIENT, color);
  mesh_colors->ambient = glm::vec3(color.r,color.g,color.b);
  material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
  mesh_colors->albedo = glm::vec3(color.r,color.g,color.b);
  material->Get(AI_MATKEY_COLOR_SPECULAR, color);
  mesh_colors->specularity = 1.0f;//glm::normalize(glm::vec3(color.r,color.g,color.b));
  material->Get(AI_MATKEY_SHININESS, shininess);
  mesh_colors->shininess = shininess;
  material->Get(AI_MATKEY_SHININESS_STRENGTH, shininess);
  mesh_colors->metalness = 0.0f;
  //mesh_colors.specular /= shininess;

  /*qDebug() << mesh->mName.C_Str();
  qDebug() << "ambient:" << mesh_colors.ambient.x << mesh_colors.ambient.y << mesh_colors.ambient.z;
  qDebug() << "diffuse:" << mesh_colors.diffuse.x << mesh_colors.diffuse.y << mesh_colors.diffuse.z;
  qDebug() << "specular" << mesh_colors.specular.x << mesh_colors.specular.y << mesh_colors.specular.z;
  qDebug() << "shininess" << mesh_colors.shininess;
  qDebug() << "shininess strength" << shininess << '\n';*/

  return new Mesh(vertices, indices, mesh_colors, transformation);
}

std::vector<Texture> Model::load_material_textures(aiMaterial *mat, aiTextureType type, std::string type_name) {
  std::vector<Texture> textures;
  for (unsigned int i=0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    bool skip = false;

    // Check if the texture is already loaded
    for (unsigned int j=0; j < textures_loaded.size(); j++) {
      if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(textures_loaded[j]);
        skip = true;
        break;
      }
    }

    if (!skip) {
      Texture texture;
      texture.id = load_texture(str.C_Str(), directory);
      texture.type = type_name;
      texture.path = str.C_Str();

      textures.push_back(texture);
      textures_loaded.push_back(texture);
    }
  }
  return textures;
}

unsigned int Model::load_texture(const char *path, std::string directory) {
  std::string filename = directory + '/' + path;

  unsigned int textureID;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  QImage img = QImage(filename.c_str()).convertToFormat(QImage::Format_RGB888);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, img.width(), img.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, img.bits());
  glGenerateMipmap(GL_TEXTURE_2D);

  return textureID;
}
