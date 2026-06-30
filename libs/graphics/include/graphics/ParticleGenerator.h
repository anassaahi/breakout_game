#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include <graphics/Shader.h>
#include <graphics/Texture2D.h>
#include <game/GameObject.h>

// A single particle: where it is, how it's moving, what color/alpha it
// currently has, and how much "life" it has left before it dies.
struct Particle
{
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    float     Life;

    Particle()
        : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) {}
};

// Owns a fixed-size pool of particles and recycles dead ones instead of
// constantly allocating new particles. Spawns a few new particles per frame
// at a given GameObject's position (here: the ball), and renders all the
// still-alive ones as small additively-blended glowing quads.
class ParticleGenerator
{
public:
    // amount — total pool size (e.g. 500). Larger pools support more
    // particles alive at once but cost a bit more per-frame update work.
    ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);

    // Spawns `newParticles` new particles at `object`'s position (offset by
    // `offset`) and ages every currently-alive particle by dt.
    void Update(float dt, GameObject& object, unsigned int newParticles,
               glm::vec2 offset = glm::vec2(0.0f, 0.0f));

    // Draws every particle that's still alive, using additive blending
    // for the glow effect.
    void Draw();

private:
    std::vector<Particle> particles;
    unsigned int           amount;

    Shader      shader;
    Texture2D   texture;
    unsigned int VAO;

    void init();

    // Returns the index of a dead particle to recycle, starting the search
    // from where the last one was found (usually near-instant).
    unsigned int firstUnusedParticle();

    void respawnParticle(Particle& particle, GameObject& object,
                         glm::vec2 offset = glm::vec2(0.0f, 0.0f));
};
