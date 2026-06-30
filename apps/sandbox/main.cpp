#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <game/Game.h>

// ── Window dimensions ────────────────────────────────────────────────────────
const unsigned int SCR_WIDTH  = 800;
const unsigned int SCR_HEIGHT = 600;

// ── Game instance ────────────────────────────────────────────────────────────
Game Breakout(SCR_WIDTH, SCR_HEIGHT);

// ── GLFW callbacks ───────────────────────────────────────────────────────────
void key_callback(GLFWwindow* window, int key, int /*scancode*/,
                  int action, int /*mode*/)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS)   Breakout.Keys[key] = true;
        if (action == GLFW_RELEASE) Breakout.Keys[key] = false;
    }
}

void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ── main ─────────────────────────────────────────────────────────────────────
int main()
{
    // Init GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
                                          "Breakout", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load OpenGL function pointers via GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialise GLAD\n";
        return -1;
    }

    // OpenGL state — 2D game needs blending, not depth testing
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialise game (loads shaders, textures, levels)
    Breakout.Init();

    // Delta-time tracking
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // ── Render loop ──────────────────────────────────────────────────────────
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        Breakout.ProcessInput(deltaTime);
        Breakout.Update(deltaTime);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        Breakout.Render();

        glfwSwapBuffers(window);
    }

    // Cleanup (Game destructor frees GPU resources via ResourceManager::Clear)
    glfwTerminate();
    return 0;
}