#include <graphics/ResourceManager.h>

#include <iostream>
#include <sstream>
#include <fstream>

// stb_image — define the implementation exactly once (here, not in a header)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ── Static member definitions ────────────────────────────────────────────────
std::map<std::string, Shader>    ResourceManager::Shaders;
std::map<std::string, Texture2D> ResourceManager::Textures;

// ── Public API ───────────────────────────────────────────────────────────────

Shader ResourceManager::LoadShader(const char* vShaderFile,
                                    const char* fShaderFile,
                                    const char* gShaderFile,
                                    const std::string& name)
{
    Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
    return Shaders[name];
}

Shader& ResourceManager::GetShader(const std::string& name)
{
    return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const char* file,
                                        bool alpha,
                                        const std::string& name)
{
    Textures[name] = loadTextureFromFile(file, alpha);
    return Textures[name];
}

Texture2D& ResourceManager::GetTexture(const std::string& name)
{
    return Textures[name];
}

void ResourceManager::Clear()
{
    for (auto& iter : Shaders)
        glDeleteProgram(iter.second.ID);
    for (auto& iter : Textures)
        glDeleteTextures(1, &iter.second.ID);
}

// ── Private helpers ──────────────────────────────────────────────────────────

Shader ResourceManager::loadShaderFromFile(const char* vPath,
                                            const char* fPath,
                                            const char* gPath)
{
    // Read source files into strings
    std::string vertexCode, fragmentCode, geometryCode;
    try {
        std::ifstream vFile(vPath), fFile(fPath);
        std::stringstream vStream, fStream;
        vStream << vFile.rdbuf();
        fStream << fFile.rdbuf();
        vertexCode   = vStream.str();
        fragmentCode = fStream.str();
        if (gPath) {
            std::ifstream gFile(gPath);
            std::stringstream gStream;
            gStream << gFile.rdbuf();
            geometryCode = gStream.str();
        }
    }
    catch (std::exception& e) {
        std::cerr << "ERROR::RESOURCE_MANAGER: failed to read shader files\n"
                  << e.what() << '\n';
    }

    const char* vSrc = vertexCode.c_str();
    const char* fSrc = fragmentCode.c_str();
    const char* gSrc = gPath ? geometryCode.c_str() : nullptr;

    Shader shader;
    shader.Compile(vSrc, fSrc, gSrc);
    return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const char* file, bool alpha)
{
    Texture2D texture;
    if (alpha) {
        texture.Internal_Format = GL_RGBA;
        texture.Image_Format    = GL_RGBA;
    }

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);
    if (!data)
        std::cerr << "ERROR::RESOURCE_MANAGER: failed to load texture: "
                  << file << '\n';

    texture.Generate(width, height, data);
    stbi_image_free(data);
    return texture;
}
