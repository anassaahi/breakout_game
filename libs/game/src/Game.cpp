#include <game/Game.h>
#include <graphics/ResourceManager.h>
#include <graphics/SpriteRenderer.h>
#include <game/Collision.h>
#include <graphics/ParticleGenerator.h>

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>   // std::abs
#include <tuple>

// ── Constants ────────────────────────────────────────────────────────────────

const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
const float     PLAYER_VELOCITY(500.0f);

const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
const float      BALL_RADIUS = 12.5f;

// ── Module-level objects ─────────────────────────────────────────────────────

SpriteRenderer*    Renderer  = nullptr;
GameObject*         Player    = nullptr;
BallObject*         Ball      = nullptr;
ParticleGenerator*  Particles = nullptr;

// ── Constructor / Destructor ─────────────────────────────────────────────────

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_ACTIVE), Keys(), Width(width), Height(height), Level(0) {}

Game::~Game()
{
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
    ResourceManager::Clear();
}

// ── Init ─────────────────────────────────────────────────────────────────────

void Game::Init()
{
    // Sprite shader + projection
    ResourceManager::LoadShader("shaders/sprite.vs", "shaders/sprite.fs",
                                nullptr, "sprite");

    glm::mat4 projection = glm::ortho(
        0.0f, static_cast<float>(Width),
        static_cast<float>(Height), 0.0f,
        -1.0f, 1.0f
    );

    ResourceManager::GetShader("sprite").Use();
    ResourceManager::GetShader("sprite").SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);

    // Particle shader + projection (needs the same ortho projection)
    ResourceManager::LoadShader("shaders/particle.vs", "shaders/particle.frag",
                                nullptr, "particle");
    ResourceManager::GetShader("particle").Use();
    ResourceManager::GetShader("particle").SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);

    Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

    // Textures
    ResourceManager::LoadTexture("textures/background.jpg",  false, "background");
    ResourceManager::LoadTexture("textures/awesomeface.png", true,  "face");
    ResourceManager::LoadTexture("textures/block.png",       false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/paddle.png",      true,  "paddle");
    ResourceManager::LoadTexture("textures/particle.png",    true,  "particle");

    // Particle generator — 500-particle pool, trailing the ball
    Particles = new ParticleGenerator(
        ResourceManager::GetShader("particle"),
        ResourceManager::GetTexture("particle"),
        500
    );

    // Levels
    GameLevel one, two, three, four;
    one.Load("levels/one.lvl",     Width, Height / 2);
    two.Load("levels/two.lvl",     Width, Height / 2);
    three.Load("levels/three.lvl", Width, Height / 2);
    four.Load("levels/four.lvl",   Width, Height / 2);

    Levels.push_back(one);
    Levels.push_back(two);
    Levels.push_back(three);
    Levels.push_back(four);
    Level = 0;

    // Paddle
    glm::vec2 playerPos(
        Width / 2.0f - PLAYER_SIZE.x / 2.0f,
        Height - PLAYER_SIZE.y
    );
    Player = new GameObject(playerPos, PLAYER_SIZE,
                            ResourceManager::GetTexture("paddle"));

    // Ball
    glm::vec2 ballPos = playerPos + glm::vec2(
        PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
        -BALL_RADIUS * 2.0f
    );
    Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY,
                          ResourceManager::GetTexture("face"));
}

// ── Input ────────────────────────────────────────────────────────────────────

void Game::ProcessInput(float dt)
{
    if (State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;

        if (Keys[GLFW_KEY_A])
        {
            if (Player->Position.x >= 0.0f)
            {
                Player->Position.x -= velocity;
                if (Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }
        if (Keys[GLFW_KEY_D])
        {
            if (Player->Position.x <= Width - Player->Size.x)
            {
                Player->Position.x += velocity;
                if (Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }
        if (Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
    }
}

// ── Update ───────────────────────────────────────────────────────────────────

void Game::Update(float dt)
{
    Ball->Move(dt, Width);

    DoCollisions();

    // Spawn 2 particles per frame at the ball's position, offset slightly
    // toward its center so they appear to emanate from inside the ball
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));

    if (Ball->Position.y >= Height)
    {
        ResetLevel();
        ResetPlayer();
    }
}

// ── Collisions ───────────────────────────────────────────────────────────────

void Game::DoCollisions()
{
    for (GameObject& box : Levels[Level].Bricks)
    {
        if (!box.Destroyed)
        {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision))
            {
                if (!box.IsSolid)
                    box.Destroyed = true;

                Direction dir       = std::get<1>(collision);
                glm::vec2 diff_vec  = std::get<2>(collision);

                if (dir == LEFT || dir == RIGHT)
                {
                    Ball->Velocity.x = -Ball->Velocity.x;

                    float penetration = Ball->Radius - std::abs(diff_vec.x);
                    if (dir == LEFT)
                        Ball->Position.x += penetration;
                    else
                        Ball->Position.x -= penetration;
                }
                else
                {
                    Ball->Velocity.y = -Ball->Velocity.y;

                    float penetration = Ball->Radius - std::abs(diff_vec.y);
                    if (dir == UP)
                        Ball->Position.y -= penetration;
                    else
                        Ball->Position.y += penetration;
                }
            }
        }
    }

    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result))
    {
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance     = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage   = distance / (Player->Size.x / 2.0f);

        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        Ball->Velocity.y = -1.0f * std::abs(Ball->Velocity.y);
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
    }
}

// ── Reset ────────────────────────────────────────────────────────────────────

void Game::ResetLevel()
{
    if (Level == 0)      Levels[0].Load("levels/one.lvl",   Width, Height / 2);
    else if (Level == 1) Levels[1].Load("levels/two.lvl",   Width, Height / 2);
    else if (Level == 2) Levels[2].Load("levels/three.lvl", Width, Height / 2);
    else if (Level == 3) Levels[3].Load("levels/four.lvl",  Width, Height / 2);
}

void Game::ResetPlayer()
{
    Player->Size     = PLAYER_SIZE;
    Player->Position = glm::vec2(
        Width / 2.0f - PLAYER_SIZE.x / 2.0f,
        Height - PLAYER_SIZE.y
    );

    Ball->Reset(
        Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS,
                                     -(BALL_RADIUS * 2.0f)),
        INITIAL_BALL_VELOCITY
    );
}

// ── Render ───────────────────────────────────────────────────────────────────

void Game::Render()
{
    if (State == GAME_ACTIVE)
    {
        Renderer->DrawSprite(
            ResourceManager::GetTexture("background"),
            glm::vec2(0.0f, 0.0f),
            glm::vec2(Width, Height),
            0.0f
        );

        Levels[Level].Draw(*Renderer);
        Player->Draw(*Renderer);

        // Particles render after the paddle but before the ball, so the
        // trail appears to come from behind/around the ball, not on top of it
        Particles->Draw();

        Ball->Draw(*Renderer);
    }
}