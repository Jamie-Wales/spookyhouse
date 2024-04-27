//
// Created by jamielinux on 2/18/24.
//

#ifndef MODELS_MESH_H
#define MODELS_MESH_H


#include "BoundingBox.h"
#include "Shader.h"
#include <assimp/mesh.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <vector>
struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoordinates;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    int boneid[4];
    float weights[4];
};

struct texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    std::vector<vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<texture> textures;
    Mesh(std::vector<vertex> vertices, std::vector<unsigned int> indices, std::vector<texture> textures, aiAABB boundingbox);
    void draw(Shader& shader);
    BoundingBox boundingbox;
    unsigned int VAO, VBO, EBO;

private:
    void setUpMesh();
};

#endif // MODELS_MESH_H
