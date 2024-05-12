#ifndef MODEL_H
#define MODEL_H
#include "Mesh.h"
#include "TextureUtils.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <memory>
#include <vector>

class Drawable {
public:
    virtual void draw(Shader& shader) = 0;
    glm::vec3 position;
    std::shared_ptr<BoundingBox> boundingbox;
    glm::mat4 translation;
    float pitch;
    float yaw;
    float roll;
    bool isInstanced = false;
    int instanceAmount = 0;
};

class Model : public Drawable {
public:
    std::vector<texture> textures_loaded;
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;
    glm::mat4 translation;
    std::shared_ptr<BoundingBox> boundingbox;
    glm::vec3 position;
    glm::vec3 origin = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::mat4 rotation;
    float pitch = 0;
    float yaw = 0;
    int id;
    float scale = 1.0f;
    float roll = 0;

    Model(const std::string& path, glm::mat4 translation, glm::vec3 position, int id, float pitch, float yaw, float roll, bool gamma = false);

    void initInstanced(size_t amount, std::vector<glm::mat4> trans);
    void drawInstanced(Shader& shader);
    void setScale(float scale);
    void addPitch(float pitch);
    void addYaw(float yaw);
    void setOrigin(glm::vec3 origin);
    void draw(Shader& shader) override;
    glm::vec3& getPosition();
    void setPosition(glm::vec3 position);

private:
    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName);
};

#endif // MODEL_H
