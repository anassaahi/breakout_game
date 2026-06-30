#include <graphics/ParticleGenerator.h>
#include <cstdlib>   // rand()

ParticleGenerator::ParticleGenerator(Shader shader, Texture2D texture,
                                     unsigned int amount)
    : shader(shader), texture(texture), amount(amount)
{
    init();
}

void ParticleGenerator::Update(float dt, GameObject& object,
                               unsigned int newParticles, glm::vec2 offset)
{
    // Spawn a handful of new particles this frame, recycling dead slots
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        unsigned int unusedParticle = firstUnusedParticle();
        respawnParticle(particles[unusedParticle], object, offset);
    }

    // Age every particle
    for (unsigned int i = 0; i < amount; ++i)
    {
        Particle& p = particles[i];
        p.Life -= dt;
        if (p.Life > 0.0f)
        {
            p.Position -= p.Velocity * dt;
            p.Color.a  -= dt * 2.5f;
        }
    }
}

void ParticleGenerator::Draw()
{
    // Additive blending gives overlapping particles a glow instead of
    // just stacking flat transparency.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    shader.Use();

    for (Particle& particle : particles)
    {
        if (particle.Life > 0.0f)
        {
            shader.SetVector2f("offset", particle.Position);
            shader.SetVector4f("color", particle.Color);
            texture.Bind();
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }

    // Restore the normal alpha blending used by every other sprite
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::init()
{
    unsigned int VBO;
    float particle_quad[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Pre-fill the pool with dead (Life == 0) particles
    for (unsigned int i = 0; i < amount; ++i)
        particles.push_back(Particle());
}

unsigned int ParticleGenerator::firstUnusedParticle()
{
    static unsigned int lastUsedParticle = 0;

    // Search forward from the last spot we found — usually near-instant
    for (unsigned int i = lastUsedParticle; i < amount; ++i)
    {
        if (particles[i].Life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }
    // Fallback: linear search from the start
    for (unsigned int i = 0; i < lastUsedParticle; ++i)
    {
        if (particles[i].Life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }
    // All particles are alive — overwrite the first one.
    // (If you hit this often, raise `amount` or shorten particle lifetime.)
    lastUsedParticle = 0;
    return 0;
}

void ParticleGenerator::respawnParticle(Particle& particle, GameObject& object,
                                        glm::vec2 offset)
{
    float random  = ((rand() % 100) - 50) / 10.0f;
    float rColor  = 0.5f + ((rand() % 100) / 100.0f);

    particle.Position = object.Position + random + offset;
    particle.Color    = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.Life     = 1.0f;
    particle.Velocity = object.Velocity * 0.1f;
}
