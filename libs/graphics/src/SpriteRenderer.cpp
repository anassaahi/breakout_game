#include <graphics/SpriteRenderer.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

SpriteRenderer::SpriteRenderer(Shader& shader)
    : shader(shader)
{
    initRenderData();
}

SpriteRenderer::~SpriteRenderer()
{
    glDeleteVertexArrays(1, &quadVAO);
}

void SpriteRenderer::DrawSprite(Texture2D& texture,
                                 glm::vec2  position,
                                 glm::vec2  size,
                                 float      rotate,
                                 glm::vec3  color)
{
    shader.Use();

    // Build the model matrix:
    //   1. Translate to position
    //   2. Move origin to centre of sprite (so rotation is around the centre)
    //   3. Rotate
    //   4. Move origin back to top-left
    //   5. Scale to desired size
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(position, 0.0f));

    // Rotate around the sprite's centre
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));

    model = glm::scale(model, glm::vec3(size, 1.0f));

    // Upload uniforms
    shader.SetMatrix4("model", model);
    shader.SetVector3f("spriteColor", color);

    // Bind texture to unit 0
    glActiveTexture(GL_TEXTURE0);
    texture.Bind();

    // Draw the quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SpriteRenderer::initRenderData()
{
    // A unit quad (0,0)→(1,1) in two triangles.
    // Layout: x, y, texU, texV
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    unsigned int VBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // location 0: vec4(x, y, texU, texV)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
