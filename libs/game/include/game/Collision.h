#pragma once
#include <tuple>
#include <glm/glm.hpp>

#include <game/GameObject.h>
#include <game/BallObject.h>

// Which side of an AABB the ball hit. Determined via the "closest compass
// direction" trick: dot-product the penetration vector against up/right/
// down/left and pick whichever gives the largest (closest to 1.0) result.
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT
};

// (didCollide, side hit, penetration vector from AABB-edge to ball-center)
typedef std::tuple<bool, Direction, glm::vec2> Collision;

// Plain AABB-AABB overlap test — true if the two rectangles intersect at all.
// Not used for the ball anymore (see the circle version below) but kept
// since it's the simplest way to test brick-vs-brick or paddle-vs-wall style
// checks if you need them later.
bool CheckCollision(GameObject& one, GameObject& two);

// AABB-circle test, ball vs. any rectangular GameObject (brick or paddle).
// Returns whether they collided, which side of the AABB was hit, and the
// vector needed to push the ball back out of the AABB.
Collision CheckCollision(BallObject& one, GameObject& two);

// Picks the compass direction (UP/RIGHT/DOWN/LEFT) closest in angle to `target`.
Direction VectorDirection(glm::vec2 target);
