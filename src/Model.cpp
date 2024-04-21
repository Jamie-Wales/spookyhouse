//
// Created by jamielinux on 2/18/24.
//

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifndef CONTEXT_H
#define CONTEXT_H
#define MINIAUDIO_IMPLEMENTATION
#include <lib/miniaudo.h>

#endif
// void Model::draw(Shader &shader) {
//     for (Mesh mesh : meshes) {
//         mesh.draw(shader);
//     }
// }
//
// void Model::loadModel(std::string path) {
//     Assimp::Importer import;
//     const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
//     if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
//         std::cerr << "Assimp error importing file, "  << import.GetErrorString() << std::endl;
//         return;
//     }
//
//     directory = path.substr(0, path.find_last_of('/'));
//     processNode(scene->mRootNode, scene);
// }
//
//
// void Model::processNode(aiNode *node, const aiScene *scene) {
//     for(unsigned int i = 0; i < node->mNumMeshes; i++)
//     {
//         aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
//         meshes.push_back(processMesh(mesh, scene));
//     }
//     for(unsigned int i = 0; i < node->mNumChildren; i++)
//     {
//         processNode(node->mChildren[i], scene);
//     }
//
// }
//
//
// Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
//     std::vector<vertex> vertices;
//     std::vector<unsigned int> indices;
//     std::vector<texture> textures;
//
//     // walk through each of the mesh's vertices
//     for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
//         vertex vertex;
//         glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
//         // positions
//         vector.x = mesh->mVertices[i].x;
//         vector.y = mesh->mVertices[i].y;
//         vector.z = mesh->mVertices[i].z;
//         vertex.position = vector;
//         // normals
//         if (mesh->HasNormals()) {
//             vector.x = mesh->mNormals[i].x;
//             vector.y = mesh->mNormals[i].y;
//             vector.z = mesh->mNormals[i].z;
//             vertex.normal = vector;
//         }
//         // texture coordinates
//         if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
//         {
//             glm::vec2 vec;
//             vec.x = mesh->mTextureCoords[0][i].x;
//             vec.y = mesh->mTextureCoords[0][i].y;
//             vertex.textureCoordinates = vec;
//             // tangent
//             // vector.x = mesh->mTangents[i].x;
//              //vector.y = mesh->mTangents[i].y;
////            vector.z = mesh->mTangents[i].z;
////            vertex.tangent = vector;
////            // bitangent
////            vector.x = mesh->mBitangents[i].x;
////            vector.y = mesh->mBitangents[i].y;
////             vector.z = mesh->mBitangents[i].z;
////            vertex.bitangent = vector;
//        } else {
//            vertex.textureCoordinates = glm::vec2(0.0f, 0.0f);
//        }
//
//        vertices.push_back(vertex);
//    }
//    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
//        aiFace face = mesh->mFaces[i];
//        for (unsigned int j = 0; j < face.mNumIndices; j++) {
//            indices.push_back(face.mIndices[j]);
//        }
//    }
//    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
//
//    // 1. diffuse maps
//    std::vector<texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
//    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
//    // 2. specular maps
//    std::vector<texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
//    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
//    // 3. normal maps
//    //std::vector<texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
//    //textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
//    // 4. height maps
//   // std::vector<texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
//    //textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
//
//    // return a mesh object created from the extracted mesh data
//    return {vertices, indices, textures};
//}
//
// std::vector<texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName) {
//    std::vector<texture> textures;
//    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
//        aiString str;
//        mat->GetTexture(type, i, &str);
//        bool skip = false;
//        for (unsigned int j = 0; j < textures.size(); j++) {
//            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
//                textures.push_back(textures_loaded[j]);
//                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
//                break;
//            }
//        }
//        if (!skip) {   // if texture hasn't been loaded already, load it
//            texture texture;
//            texture.id = textureFromFile(str.C_Str(), this->directory);
//            texture.type = typeName;
//            texture.path = str.C_Str();
//            textures.push_back(texture);
//            textures_loaded.push_back(
//                    texture);  // store it as texture loaded for entire model, to ensure we won't load duplicate textures.
//        }
//    }
//    return textures;
//}
// unsigned int Model::textureFromFile(const char *path, const std::string &directory) {
//    std::string filename = std::string(path);
//    filename = directory + '/' + filename;
//
//    unsigned int textureID;
//    glGenTextures(1, &textureID);
//
//    int width, height, nrComponents;
//    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
//    if (data)
//    {
//        GLenum format;
//        if (nrComponents == 1)
//            format = GL_RED;
//        else if (nrComponents == 3)
//            format = GL_RGB;
//        else if (nrComponents == 4)
//            format = GL_RGBA;
//
//        glBindTexture(GL_TEXTURE_2D, textureID);
//        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//        glGenerateMipmap(GL_TEXTURE_2D);
//
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//
//    }
//    else
//    {
//        std::cout << "Texture failed to load at path: " << path << std::endl;
//        stbi_image_free(data);
//    }
//
//    return textureID;
//}
