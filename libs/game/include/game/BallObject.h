#pragma once
#include <glm/glm.hpp>
#include <game/GameObject.h>

// The ball is a GameObject with two extra bits of state: a radius
// (used for nothing yet, but needed once we do circle-vs-AABB collision
// in the next chapter) and Stuck, which keeps it glued to the paddle
// until the player presses space.
class BallObject : public GameObject
{
public:
    float Radius;
    bool  Stuck;

    BallObject()
        : GameObject(), Radius(12.5f), Stuck(true) {}

    BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite)
        : GameObject(pos, glm::vec2(radius * 2.0f, radius * 2.0f), sprite,
                      glm::vec3(1.0f), velocity),
          Radius(radius), Stuck(true) {}

    // Moves the ball by Velocity * dt, then bounces it off the left,
    // right, and top edges of a window of the given width.
    // (No bottom-edge bounce — reaching the bottom means game over / lose a life,
    //  handled elsewhere.)
    glm::vec2 Move(float dt, unsigned int window_width)
    {
        if (!Stuck)
        {
            Position += Velocity * dt;

            if (Position.x <= 0.0f)
            {
                Velocity.x = -Velocity.x;
                Position.x = 0.0f;
            }
            else if (Position.x + Size.x >= window_width)
            {
                Velocity.x = -Velocity.x;
                Position.x = window_width - Size.x;
            }

            if (Position.y <= 0.0f)
            {
                Velocity.y = -Velocity.y;
                Position.y = 0.0f;
            }
        }
        return Position;
    }

    // Re-sticks the ball at a new position/velocity (used after losing a life).
    void Reset(glm::vec2 position, glm::vec2 velocity)
    {
        Position = position;
        Velocity = velocity;
        Stuck    = true;
    }
};
