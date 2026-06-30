#pragma once
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <game/GameLevel.h>
#include <game/GameObject.h>
#include <game/BallObject.h>

enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_WIN
};

class Game
{
public:
    GameState    State;
    bool         Keys[1024];
    unsigned int Width, Height;

    std::vector<GameLevel> Levels;
    unsigned int            Level;

    Game(unsigned int width, unsigned int height);
    ~Game();

    void Init();
    void ProcessInput(float dt);
    void Update(float dt);
    void Render();

    void DoCollisions();

    void ResetLevel();
    void ResetPlayer();
};