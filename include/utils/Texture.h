//
// Created by Jamie Wales on 27/04/2024.
//

#ifndef SPOOKY_TEXTURE_H
#define SPOOKY_TEXTURE_H
#include <gl/glew.h>
#include <string>

class Texture {
public:
    Texture(GLenum TextureTarget, const std::string& FileName);

    Texture(GLenum TextureTarget);

    // Should be called once to load the texture
    bool Load();

    void Load(unsigned int bufferSize, void* pImageData);

    void Load(const std::string& Filename);

    void LoadRaw(int Width, int Height, int BPP, const unsigned char* pImageData);

    void LoadF32(int Width, int Height, const float* pImageData);

    // Must be called at least once for the specific texture unit
    void Bind(GLenum TextureUnit);

    void GetImageSize(int& ImageWidth, int& ImageHeight)
    {
        ImageWidth = m_imageWidth;
        ImageHeight = m_imageHeight;
    }

    GLuint GetTexture() const { return m_textureObj; }

private:
    void LoadInternal(const void* pImageData);
    void LoadInternalNonDSA(const void* pImageData);
    void LoadInternalDSA(const void* pImageData);

    void BindInternalNonDSA(GLenum TextureUnit);
    void BindInternalDSA(GLenum TextureUnit);

    std::string m_fileName;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_imageBPP = 0;
};
#endif // SPOOKY_TEXTURE_H
