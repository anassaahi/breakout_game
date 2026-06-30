#pragma once
#include <map>
#include <string>
#include <glad/glad.h>

#include <graphics/Texture2D.h>
#include <graphics/Shader.h>   // your existing Shader.h — keep it unchanged

// Singleton that owns every shader and texture loaded during the game.
// Access everything via ResourceManager::LoadShader(...) / GetShader(...)
class ResourceManager
{
public:
    // Storage
    static std::map<std::string, Shader>    Shaders;
    static std::map<std::string, Texture2D> Textures;

    // Load (compile + link) a shader from file paths and store it under 'name'.
    // geometryPath may be nullptr if you have no geometry shader.
    static Shader LoadShader(const char* vShaderFile,
                             const char* fShaderFile,
                             const char* gShaderFile,
                             const std::string& name);

    static Shader& GetShader(const std::string& name);

    // Load a texture from an image file and store it under 'name'.
    // Set alpha = true for PNG files that have transparency.
    static Texture2D LoadTexture(const char* file,
                                 bool alpha,
                                 const std::string& name);

    static Texture2D& GetTexture(const std::string& name);

    // Free all loaded resources (call on shutdown).
    static void Clear();

private:
    ResourceManager() {}   // pure static — never instantiated

    // Internal helpers that do the real work
    static Shader    loadShaderFromFile(const char* vPath,
                                        const char* fPath,
                                        const char* gPath = nullptr);
    static Texture2D loadTextureFromFile(const char* file, bool alpha);
};
