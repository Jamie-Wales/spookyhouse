#include "Terrain.h"
#include "./utils/Utilities.h"
#include "./utils/Vertex.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <initializer_list>

void getRandomPoint(TerrainPoint& p1, TerrainPoint& p2, int terrainSize)
{
    p1.x = rand() % terrainSize;
    p1.z = rand() % terrainSize;
    p2.x = rand() % terrainSize;
    p2.z = rand() % terrainSize;
    int count = 0;
    do {
        p2.x = rand() % terrainSize;
        p2.z = rand() % terrainSize;
        count++;
    } while (p1.isEqual(p2));
}

void Terrain::faultFormationTerrain(int iterations, float minHeight, float maxHeight)
{
    float deltaHeight = maxHeight - minHeight;

    for (int i = 0; i < iterations; i++) {

        float iterationRatio = ((float)i / (float)iterations);
        float height = maxHeight - iterationRatio * deltaHeight;

        TerrainPoint p1;
        TerrainPoint p2;
        getRandomPoint(p1, p2, this->terrainSize);
        int dirx = p2.x - p1.x;
        int dirz = p2.z - p1.z;
        for (int z = 0; z < this->terrainSize; z++) {
            for (int x = 0; x < this->terrainSize; x++) {
                int dirx_in = x - p1.x;
                int dirz_in = z - p1.z;
                int cross = dirx_in * dirz - dirx * dirz_in;
                if (cross > 0) {
                    float currentHeight = heightMap(z, x);
                    heightMap(z, x) = currentHeight + height;
                }
            }
        }
    }

    ApplyFirFilter(0.8);
}

void Terrain::CreateFaultFormation(int terrainSize, int iterations, float minHeight, float maxHeight)
{
    this->terrainSize = terrainSize;
    this->minHeight = minHeight;
    this->maxHeight = maxHeight;
    this->height = terrainSize;
    this->width = terrainSize;
    this->heightMap = Array2D<float>(terrainSize, terrainSize, 0.0f);
    faultFormationTerrain(iterations, minHeight, maxHeight);
    heightMap.normalize(minHeight, maxHeight);
}

Terrain::Terrain(float scale, std::initializer_list<const std::string> textureFiles, float textureScale)
    : scale(scale)
    , textureScale(textureScale)
{

    terposition = glm::vec3(0.0);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    int position = 0;
    int tex = 1;
    int normal = 2;
    size_t numfloats = 0;
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(numfloats * sizeof(float)));
    numfloats += 3;
    glEnableVertexAttribArray(tex);
    glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(numfloats * sizeof(float)));
    numfloats += 2;
    glEnableVertexAttribArray(normal);
    glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(numfloats * sizeof(float)));
    numfloats += 3;
    CreateFaultFormation(500, 100, 1, 100);
    populateBuffer();

    for (auto& textureFile : textureFiles) {
        Texture texture(GL_TEXTURE_2D, textureFile);
        texture.Load();
        textures.push_back(std::make_shared<Texture>(texture));
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Terrain::LoadFromFile(const std::string& filename)
{
    int fileSize = 0;
    char* data = readBinaryFile(*filename.c_str(), fileSize);
    if (data == nullptr) {
        return;
    }
    auto* ptr = reinterpret_cast<unsigned char*>(data);
    terrainSize = sqrt((fileSize / sizeof(float)));
    heightMap = Array2D<float>(terrainSize, terrainSize, (float*)ptr);
    height = terrainSize;
    width = terrainSize;
    delete[] data;
}

float Terrain::getHeight(int x, int z) const
{
    return heightMap(x, z);
}

float Terrain::getWorldHeight()
{
    return this->terrainSize / this->scale;
}

float Terrain::FIRFilterSinglePoint(int x, int z, float preval, float filter)
{
    float curVal = heightMap(x, z);
    float newVal = filter * preval + (1 - filter) * curVal;
    heightMap(x, z) = newVal;
    return newVal;
}

void Terrain::ApplyFirFilter(float filter)
{
    for (int z = 0; z < terrainSize; z++) {
        float PrevVal = heightMap(0, z);
        for (int x = 1; x < terrainSize; x++) {
            PrevVal = FIRFilterSinglePoint(x, z, PrevVal, filter);
        }
    }

    // right to left
    for (int z = 0; z < terrainSize; z++) {
        float PrevVal = heightMap(terrainSize - 1, z);
        for (int x = terrainSize - 2; x >= 0; x--) {
            PrevVal = FIRFilterSinglePoint(x, z, PrevVal, filter);
        }
    }

    // bottom to top
    for (int x = 0; x < terrainSize; x++) {
        float PrevVal = heightMap(x, 0);
        for (int z = 1; z < terrainSize; z++) {
            PrevVal = FIRFilterSinglePoint(x, z, PrevVal, filter);
        }
    }

    // top to bottom
    for (int x = 0; x < terrainSize; x++) {
        float PrevVal = heightMap(x, terrainSize - 1);
        for (int z = terrainSize - 2; z >= 0; z--) {
            PrevVal = FIRFilterSinglePoint(x, z, PrevVal, filter);
        }
    }
}

void Terrain::populateBuffer()
{
    std::vector<Vertex> vertices;
    vertices.resize(height * width);

    int index = 0;
    for (int x = 0; x < width; x++) {
        for (int z = 0; z < height; z++) {
            Vertex vertex;
            vertex.init(*this, x, z);
            vertices[index] = vertex;
            index++;
        }
    }

    std::vector<unsigned int> indices;
    int numQuads = (width - 1) * (height - 1);
    indices.resize(numQuads * 6);
    int i = 0;
    for (int x = 0; x < width - 1; x++) {
        for (int z = 0; z < height - 1; z++) {
            unsigned int topLeft = (z * width) + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = ((z + 1) * width) + x;
            unsigned int bottomRight = bottomLeft + 1;
            indices[i++] = topLeft;
            indices[i++] = bottomLeft;
            indices[i++] = topRight;
            indices[i++] = topRight;
            indices[i++] = bottomLeft;
            indices[i++] = bottomRight;
        }
    }
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

float Terrain::getTerPosition(int x, int z)
{
    return getHeight(x, z);
}

void Terrain::render()
{
    unsigned int a = 0;
    unsigned int b = 1;
    unsigned int c = 2;
    unsigned int d = 3;
    glUniform1i(glGetUniformLocation(terrainShader.ID, "gTextureHeight0"), a);
    glUniform1i(glGetUniformLocation(terrainShader.ID, "gTextureHeight1"), b);
    glUniform1i(glGetUniformLocation(terrainShader.ID, "gTextureHeight2"), c);
    glUniform1i(glGetUniformLocation(terrainShader.ID, "gTextureHeight3"), d);
    textures[0]->Bind(GL_TEXTURE0);
    textures[1]->Bind(GL_TEXTURE1);
    textures[2]->Bind(GL_TEXTURE2);
    textures[3]->Bind(GL_TEXTURE3);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, (width - 1) * (height - 1) * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
