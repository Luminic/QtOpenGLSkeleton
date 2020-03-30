#include <QImage>
#include <QDebug>

#include "Model.h"
#include "Scene.h"


inline glm::mat4 aiMat_to_glmMat(const aiMatrix4x4& from) {
  glm::mat4 to(
    {{from.a1,from.b1,from.c1,from.d1}
    ,{from.a2,from.b2,from.c2,from.d2}
    ,{from.a3,from.b3,from.c3,from.d3}
    ,{from.a4,from.b4,from.c4,from.d4}}
  );
  return to;
}

inline glm::vec3 aiVector3D_to_glm_vec3(const aiVector3D& from) {
  return glm::vec3(from.x, from.y, from.z);
}

inline glm::quat aiQuaternion_to_glm_quat(const aiQuaternion& from) {
  return glm::quat(from.w, from.x, from.y, from.z);
}

Model::Model(const char* path) {
  name = path;
  load_model(path);
}

Model::Model(const char* path, const char* name) {
  this->name = name;
  load_model(path);
}

Model::~Model() {
}

void Model::load_model(std::string path) {
  Assimp::Importer importer;
  const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_RemoveRedundantMaterials);
  // Check if the model loaded correctly
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    qDebug() << "ERROR::ASSIMP::" << importer.GetErrorString();
    return;
  }

  directory = path.substr(0, path.find_last_of('/'));

  process_node(scene->mRootNode, scene, true);
  load_armature(this);
  load_animations(scene);
}

Node* Model::process_node(aiNode* node, const aiScene* scene, bool root) {
  Node* my_node;
  if (root) {
    my_node = this;
  } else {
    my_node = new Node();
    my_node->root_node = this;
    my_node->name = node->mName.C_Str();
  }
  // Create a map from the node's ORIGINAL name to the node itself
  loaded_nodes[std::string(node->mName.C_Str())] = my_node;

  my_node->set_transformation(aiMat_to_glmMat(node->mTransformation));

  // Process the node's mesh (might be none)
  for (unsigned int i=0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    my_node->add_mesh(std::shared_ptr<Mesh>(process_mesh(mesh, scene)));
  }

  // Process the node's children (might be none)
  for (unsigned int i=0; i < node->mNumChildren; i++) {
    my_node->add_child_node(std::shared_ptr<Node>(process_node(node->mChildren[i], scene)));
  }

  return my_node;
}

Mesh* Model::process_mesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

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

    vertex.bone_ids = glm::ivec4(0);
    vertex.bone_weights = glm::vec4(0.0f);

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

  load_material_textures(material, mesh_colors);

  // Load colors
  aiColor3D color(0.0f,0.0f,0.0f);
  material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
  mesh_colors->color = glm::vec3(color.r,color.g,color.b);

  mesh_colors->metalness = 1.0f;

  mesh_colors = Scene::is_material_loaded(mesh_colors);

  // Create bones list
  for (unsigned int i=0; i<mesh->mNumBones; i++) {
    int bone_index;
    auto it = loaded_bones.find(std::string(mesh->mBones[i]->mName.C_Str()));
    if (it == loaded_bones.end()) {
      // The  is contained be the root node
      // Not the mesh's direct parent node (that is why I specifically use this-> even though it's unnecessary)
      bone_index = this->armature.size();
      loaded_bones[std::string(mesh->mBones[i]->mName.C_Str())] = bone_index;
      Bone bone;
      bone.offset = aiMat_to_glmMat(mesh->mBones[i]->mOffsetMatrix);
      this->armature.push_back(bone);
    } else {
      bone_index = (int) it->second;
    }
    for (unsigned int j=0; j<mesh->mBones[i]->mNumWeights; j++) {
      unsigned int vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
      float vertex_weight = mesh->mBones[i]->mWeights[j].mWeight;
      for (int n=0; n<4; n++) { // The 5th bone is intentional to make sure that if there are too many bones an error is thrown
        if (vertices[vertex_id].bone_weights[n] <= 0.001) {
          vertices[vertex_id].bone_ids[n] = bone_index;
          vertices[vertex_id].bone_weights[n] = vertex_weight;
          break;
        }
        Q_ASSERT_X(n != 4, "Bone loading", "Too many bones for one vertex");
      }
    }
  }

  Mesh *my_mesh = new Mesh(vertices, indices, mesh_colors);
  my_mesh->name = mesh->mName.C_Str();
  return my_mesh;
}

void Model::load_material_textures(aiMaterial *mat, Material *mesh_material) {
  aiString str;
  // Load diffuse maps
  for (unsigned int i=0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); i++) {
    mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
    std::string path = std::string(directory) + '/' + str.C_Str();

    mesh_material->load_texture(path.c_str(), ALBEDO_MAP);
  }
  // Load specular maps (can only load one atm--otherwise it should be the same as albedo maps)
  if (mat->GetTextureCount(aiTextureType_SPECULAR) >= 1) {
    mat->GetTexture(aiTextureType_SPECULAR, 0, &str);
    std::string path = std::string(directory) + '/' + str.C_Str();

    mesh_material->load_texture(path.c_str(), ROUGHNESS_MAP);
  }
  // Cannot load any other type of map atm
}

void Model::load_armature(Node* node) {
  auto it = this->loaded_bones.find(node->name);
  if (it != this->loaded_bones.end()) {
    node->set_bone_id(it->second);
  }

  for (auto child_node : node->get_child_nodes()) {
    load_armature(child_node.get());
  }
}

void Model::load_animations(const aiScene* scene) {
  qDebug() << scene->mNumAnimations << "animations found for" << name.c_str();
  for (unsigned int i=0; i<scene->mNumAnimations; i++) {
    float tps = scene->mAnimations[i]->mTicksPerSecond >= 0.0001 ? scene->mAnimations[i]->mTicksPerSecond : 24.0f;
    unsigned int duration = scene->mAnimations[i]->mDuration;
    std::string animation_name(scene->mAnimations[i]->mName.C_Str());

    NodeAnimation* my_animation = new NodeAnimation(tps, duration, animation_name);
    this->animation[animation_name] = my_animation;

    for (unsigned int j=0; j<scene->mAnimations[i]->mNumChannels; j++) {
      const aiNodeAnim* ai_animation = scene->mAnimations[i]->mChannels[j];

      auto it = loaded_nodes.find(std::string(ai_animation->mNodeName.C_Str()));
      Q_ASSERT_X(it != loaded_nodes.end(), "loading animations", "node specified for animation does not exist");
      it->second->set_animated(true);

      NodeAnimationChannel* my_animation_channel = new NodeAnimationChannel(std::string(ai_animation->mNodeName.C_Str()));
      for (unsigned int n=0; n<ai_animation->mNumPositionKeys; n++) {
        my_animation_channel->add_position_key(
          VectorKey{
            (float) ai_animation->mPositionKeys[n].mTime,
            aiVector3D_to_glm_vec3(ai_animation->mPositionKeys[n].mValue)
          }
        );
      }
      for (unsigned int n=0; n<ai_animation->mNumRotationKeys; n++) {
        my_animation_channel->add_rotation_key(
          QuaternionKey{
            (float) ai_animation->mRotationKeys[n].mTime,
            aiQuaternion_to_glm_quat(ai_animation->mRotationKeys[n].mValue)
          }
        );
      }
      for (unsigned int n=0; n<ai_animation->mNumScalingKeys; n++) {
        my_animation_channel->add_scale_key(
          VectorKey{
            (float) ai_animation->mScalingKeys[n].mTime,
            aiVector3D_to_glm_vec3(ai_animation->mScalingKeys[n].mValue)
          }
        );
      }

      my_animation_channel->verify();

      my_animation->animation_channels[std::string(ai_animation->mNodeName.C_Str())] = my_animation_channel;
    }
  }
}
