#pragma once
#include <vector>
#include <game/GameObject.h>
#include <graphics/SpriteRenderer.h>

// A level is just a collection of bricks (GameObjects), built from a
// text file of numbers — one row per line, one number per brick column.
//   0 = empty, 1 = solid (indestructible), 2+ = destroyable (color-coded)
class GameLevel
{
public:
    std::vector<GameObject> Bricks;

    GameLevel() {}

    // Reads `file`, then builds the bricks so the whole grid fits exactly
    // inside a levelWidth x levelHeight area.
    void Load(const char* file, unsigned int levelWidth, unsigned int levelHeight);

    void Draw(SpriteRenderer& renderer);

    // True once every non-solid brick has been destroyed.
    bool IsCompleted();

private:
    void init(std::vector<std::vector<unsigned int>> tileData,
              unsigned int levelWidth, unsigned int levelHeight);
};
