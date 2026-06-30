#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <graphics/Texture2D.h>
#include <graphics/Shader.h>

// Draws a single textured quad per DrawSprite() call.
// One VAO is shared for all sprites — only the transform changes per draw.
class SpriteRenderer
{
public:
    // Pass the shader you want used for all sprite draws.
    SpriteRenderer(Shader& shader);
    ~SpriteRenderer();

    // Draw a sprite:
    //   texture  — the Texture2D to sample
    //   position — top-left corner in screen pixels (with our ortho projection)
    //   size     — width / height in pixels  (default: 10×10)
    //   rotate   — degrees, clockwise
    //   color    — tint multiplied on top of the texture (default: white = no tint)
    void DrawSprite(Texture2D& texture,
                    glm::vec2  position,
                    glm::vec2  size   = glm::vec2(10.0f, 10.0f),
                    float      rotate = 0.0f,
                    glm::vec3  color  = glm::vec3(1.0f));

private:
    Shader&      shader;
    unsigned int quadVAO;

    void initRenderData();   // sets up the VAO once in the constructor
};
