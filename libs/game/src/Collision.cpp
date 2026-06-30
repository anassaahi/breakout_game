#include <game/Collision.h>

// ── Plain AABB - AABB ────────────────────────────────────────────────────────

bool CheckCollision(GameObject& one, GameObject& two)
{
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
                       two.Position.x + two.Size.x >= one.Position.x;
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
                       two.Position.y + two.Size.y >= one.Position.y;
    return collisionX && collisionY;
}

// ── AABB - Circle ────────────────────────────────────────────────────────────

Collision CheckCollision(BallObject& one, GameObject& two)
{
    // Ball's center
    glm::vec2 center(one.Position + one.Radius);

    // AABB's half-extents and center
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x,
                          two.Position.y + aabb_half_extents.y);

    // Closest point on the AABB to the ball's center
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    glm::vec2 closest = aabb_center + clamped;

    // Vector from closest point to the ball's center
    difference = closest - center;

    if (glm::length(difference) <= one.Radius)
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

// ── Direction helper ─────────────────────────────────────────────────────────

Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2( 0.0f,  1.0f),  // up
        glm::vec2( 1.0f,  0.0f),  // right
        glm::vec2( 0.0f, -1.0f),  // down
        glm::vec2(-1.0f,  0.0f)   // left
    };

    float max = 0.0f;
    unsigned int best_match = 0;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}
