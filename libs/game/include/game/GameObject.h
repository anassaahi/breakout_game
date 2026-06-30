#pragma once
#include <glm/glm.hpp>
#include <graphics/Texture2D.h>
#include <graphics/SpriteRenderer.h>

// Base representation of any object in the game world: position, size,
// velocity, color, rotation, and a sprite texture. Bricks, the paddle,
// and the ball are all built on top of this (the ball via BallObject).
class GameObject
{
public:
    glm::vec2 Position;
    glm::vec2 Size;
    glm::vec2 Velocity;
    glm::vec3 Color;
    float     Rotation;
    bool      IsSolid;
    bool      Destroyed;

    Texture2D Sprite;

    GameObject()
        : Position(0.0f, 0.0f), Size(1.0f, 1.0f), Velocity(0.0f, 0.0f),
          Color(1.0f), Rotation(0.0f), IsSolid(false), Destroyed(false) {}

    GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite,
               glm::vec3 color = glm::vec3(1.0f),
               glm::vec2 velocity = glm::vec2(0.0f, 0.0f))
        : Position(pos), Size(size), Velocity(velocity), Color(color),
          Rotation(0.0f), IsSolid(false), Destroyed(false), Sprite(sprite) {}

    // Draws this object's sprite using the given renderer.
    virtual void Draw(SpriteRenderer& renderer)
    {
        renderer.DrawSprite(Sprite, Position, Size, Rotation, Color);
    }
};
