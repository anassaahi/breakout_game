# Project Report: OpenGL Breakout Game Engine
**Internship Project — Graphics Programming with OpenGL**
**Author:** Ignacio Varga
**Date:** June 2026

---

## 1. Project Overview

This project is a fully functional implementation of the classic **Breakout arcade game**, built from scratch in **C++ using OpenGL 3.3 Core Profile**. The goal was not merely to recreate the game, but to build a proper **graphics engine foundation** — learning how a real-time rendering pipeline works from first principles: window creation, GPU resource management, shader programming, 2D sprite rendering, collision detection, and particle effects.

The project follows the architecture taught in the well-known [learnopengl.com](https://learnopengl.com) tutorial series by Joey de Vries, adapted and reorganised into a modular library structure suitable for a real software project.

---

## 2. Technology Stack

| Component | Technology | Version |
|---|---|---|
| Language | C++ | C++17 |
| Graphics API | OpenGL | 3.3 Core Profile |
| GL Loader | GLAD | — |
| Window/Input | GLFW | System library |
| Math Library | GLM (OpenGL Mathematics) | Bundled |
| Image Loading | stb_image | Bundled (single-header) |
| Build System | CMake | ≥ 3.10 |
| Compiler | GCC | 15.2.0 |
| OS | Linux (Ubuntu) | — |

### Why These Choices?
- **OpenGL 3.3 Core Profile** is deliberately restrictive — it forbids the old fixed-function pipeline and forces you to write shaders and manage GPU memory manually. This builds a genuine understanding of how the GPU works.
- **GLAD** is a loader generator; without it, every OpenGL function call would need to be manually resolved from the driver at runtime. GLAD does this once at startup.
- **GLFW** handles the OS-specific work of creating a window with an OpenGL context and capturing keyboard input.
- **GLM** mirrors GLSL's math types (`vec2`, `vec3`, `mat4`) in C++, making the translation between CPU logic and GPU shader code seamless.
- **CMake** enables a clean, cross-platform build description that separates the source tree from the binary output.

---

## 3. Project Structure

The project was refactored from a flat single-directory layout into a well-organised **multi-library CMake workspace**:

```
OpenGL_Project/
├── CMakeLists.txt              ← root build definition
├── apps/
│   ├── CMakeLists.txt
│   └── sandbox/
│       ├── CMakeLists.txt      ← sandbox app build + asset copy
│       └── main.cpp            ← entry point, GLFW window, game loop
├── libs/
│   ├── CMakeLists.txt
│   ├── external_libs/
│   │   ├── glad/               ← GL function loader (C source + header)
│   │   ├── glm/                ← math library (headers only)
│   │   └── stb_image/          ← image decoder (single-header)
│   ├── graphics/               ← low-level GPU abstraction layer
│   │   ├── include/graphics/
│   │   │   ├── Shader.h
│   │   │   ├── Texture2D.h
│   │   │   ├── SpriteRenderer.h
│   │   │   ├── ResourceManager.h
│   │   │   └── ParticleGenerator.h
│   │   └── src/
│   │       ├── SpriteRenderer.cpp
│   │       ├── ResourceManager.cpp
│   │       └── ParticleGenerator.cpp
│   └── game/                   ← game logic layer
│       ├── include/game/
│       │   ├── Game.h
│       │   ├── GameLevel.h
│       │   ├── GameObject.h
│       │   ├── BallObject.h
│       │   └── Collision.h
│       └── src/
│           ├── Game.cpp
│           ├── GameLevel.cpp
│           └── Collision.cpp
└── assets/
    ├── shaders/                ← GLSL shader source files
    ├── textures/               ← PNG / JPG sprite images
    └── levels/                 ← plain-text level layout files
```

### Architecture Insight
The structure follows a **dependency layering** principle:

```
apps/sandbox  →  libs/game  →  libs/graphics  →  external_libs
```

- `graphics` has zero knowledge of game logic — it only knows about GPUs.
- `game` uses `graphics` but contains all gameplay rules.
- `apps/sandbox` wires both together and owns the main loop.

This separation means the graphics layer could be reused for a completely different game without any changes.

---

## 4. Build System (CMake)

Three `CMakeLists.txt` files define the build:

### `libs/graphics/CMakeLists.txt`
Compiles all `.cpp` files in `src/` into a **static library** called `libgraphics.a`. Exposes include paths for `glad`, `glm`, `stb_image`, and its own `include/` directory, plus the game headers (needed because `ParticleGenerator` references `GameObject`). Links against system GLFW and OpenGL libraries.

### `libs/game/CMakeLists.txt`
Compiles game logic into `libgame.a`. Publicly links `libgraphics.a` so any consumer of `game` automatically inherits all graphics headers and link dependencies.

### `apps/sandbox/CMakeLists.txt`
Produces the `sandbox` executable, sets its **output directory to the build root** (`CMAKE_BINARY_DIR`), and copies the three asset folders (`shaders/`, `textures/`, `levels/`) directly there. This ensures relative file paths used in the code resolve correctly when the executable is run.

**Build commands:**
```bash
cmake -S . -B build      # configure
cmake --build build      # compile
cd build && ./sandbox    # run
```

---

## 5. Rendering Pipeline

This project implements a complete **2D orthographic rendering pipeline** on the GPU.

### 5.1 The Coordinate System

An **orthographic projection matrix** is set up at startup mapping screen pixels directly to clip space:

```cpp
glm::mat4 projection = glm::ortho(
    0.0f, (float)Width,   // left, right
    (float)Height, 0.0f,  // bottom, top  (Y flipped so (0,0) = top-left)
    -1.0f, 1.0f           // near, far
);
```

This matrix is uploaded once to every shader as a uniform. No perspective is used — the game is purely 2D.

### 5.2 Shaders (GLSL)

All shading happens on the GPU in **GLSL 3.30 Core** shaders. Two shader programs are used:

#### Sprite Shader (`sprite.vs` / `sprite.fs`)

**Vertex stage** — receives a packed `vec4(x, y, u, v)` per vertex (position + texture coordinates). Applies the `model` and `projection` matrices to transform to clip space:

```glsl
layout (location = 0) in vec4 vertex;  // <vec2 pos, vec2 tex>
out vec2 TexCoords;
uniform mat4 model;
uniform mat4 projection;

void main() {
    TexCoords   = vertex.zw;
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
```

**Fragment stage** — samples the bound texture and multiplies by a tint colour:

```glsl
uniform sampler2D image;
uniform vec3      spriteColor;

void main() {
    color = vec4(spriteColor, 1.0) * texture(image, TexCoords);
}
```

#### Particle Shader (`particle.vs` / `particle.frag`)

Similar layout but receives a `vec2 offset` (world-space position of each particle) and a `vec4 color` (with alpha that fades over the particle's lifetime). The vertex shader scales the unit quad by 10× and translates it to the particle's position. **Additive blending** (`GL_ONE`) is enabled during particle draws, which creates the glowing trail effect.

### 5.3 The GPU Quad (VAO/VBO)

The `SpriteRenderer` sets up a **single VAO + VBO** at construction time holding one unit quad (a rectangle from `(0,0)` to `(1,1)` in two triangles, 6 vertices):

```
(0,1)──(1,1)
  │  ╲    │
  │    ╲  │
(0,0)──(1,0)
```

Every sprite — background, bricks, paddle, ball — is drawn by reusing this single quad. The `model` matrix uploaded per draw transforms it into the correct position, size, and rotation. This is efficient because the GPU geometry never changes, only the uniforms do.

### 5.4 Model Matrix Construction

For each sprite draw, `SpriteRenderer::DrawSprite()` builds the model matrix step by step:

```
1. Translate to (position.x, position.y)
2. Translate to sprite centre (+0.5*size)
3. Rotate around the centre
4. Translate back (-0.5*size)
5. Scale to (width, height)
```

This order ensures rotation always happens around the sprite's own centre, not its corner.

---

## 6. Core Classes

### 6.1 `Shader` (graphics layer)

Wraps an OpenGL **shader program**. Key responsibilities:
- Compiles vertex, fragment (and optional geometry) shaders from GLSL source strings.
- Links them into a GPU program.
- Provides typed uniform setters: `SetFloat`, `SetInteger`, `SetVector2f`, `SetVector3f`, `SetVector4f`, `SetMatrix4`.
- Reports compile/link errors via `checkErrors()` with a descriptive label.

The `Compile()` method follows the standard OpenGL shader lifecycle: `glCreateShader` → `glShaderSource` → `glCompileShader` → `glCreateProgram` → `glAttachShader` → `glLinkProgram`.

### 6.2 `Texture2D` (graphics layer)

Wraps an OpenGL **texture object** (`GL_TEXTURE_2D`). Stores the format (`GL_RGB` / `GL_RGBA`), wrap mode, and filter mode. The `Generate()` method calls `glTexImage2D` to upload pixel data to the GPU and configures texture parameters.

### 6.3 `ResourceManager` (graphics layer)

A **static singleton** (no instantiation, all members are `static`) that acts as a central cache for all GPU resources:

```
ResourceManager::Shaders   →  std::map<string, Shader>
ResourceManager::Textures  →  std::map<string, Texture2D>
```

`LoadShader(vPath, fPath, gPath, name)` — reads GLSL source from disk, compiles, links, and stores under `name`.  
`LoadTexture(file, alpha, name)` — uses `stb_image` to decode the image file, then creates and uploads a `Texture2D`.  
`Clear()` — calls `glDeleteProgram` and `glDeleteTextures` on all stored resources at shutdown.

This pattern avoids loading the same shader or texture more than once, and ensures deterministic GPU resource cleanup.

### 6.4 `SpriteRenderer` (graphics layer)

Owns the single quad VAO/VBO and holds a reference to a `Shader`. `DrawSprite()` accepts a texture, position, size, rotation, and colour — builds the model matrix, uploads uniforms, binds the texture, and issues `glDrawArrays(GL_TRIANGLES, 0, 6)`.

### 6.5 `GameObject` (game layer)

The **base class** for every visible entity in the game. Stores:
- `Position` (vec2), `Size` (vec2), `Velocity` (vec2)
- `Color` (vec3), `Rotation` (float)
- `IsSolid` (bool), `Destroyed` (bool)
- `Sprite` (Texture2D — value copy)

The `Draw(SpriteRenderer&)` virtual method delegates to `SpriteRenderer::DrawSprite`. Bricks, the paddle, and (by inheritance) the ball all use this class.

### 6.6 `BallObject` (game layer)

Extends `GameObject` with:
- `Radius` (float) — used in circle–AABB collision maths.
- `Stuck` (bool) — when `true`, the ball is anchored to the paddle until the player presses Space.

`Move(dt, window_width)` advances the ball by `Velocity * dt` and bounces it off the left, right, and top walls. No bottom bounce — falling off the bottom triggers a life-loss / level reset.

`Reset(position, velocity)` re-sticks the ball at the given position (used after losing a life).

### 6.7 `GameLevel` (game layer)

Loads a level from a **plain-text file** where each number represents a tile:
- `0` → empty (skip)
- `1` → solid (indestructible) grey brick
- `2–5` → destroyable bricks with distinct colours (blue, green, yellow, orange)

`Load()` reads the file with `std::ifstream`, parses rows with `std::istringstream`, then calls the private `init()` which scales each tile to fill a `levelWidth × levelHeight` area proportionally.

`Draw(SpriteRenderer&)` iterates all bricks and draws each non-destroyed one.  
`IsCompleted()` returns `true` when no non-solid, non-destroyed bricks remain.

### 6.8 `Game` (game layer)

The **central game controller**. Owns the game state machine (`GAME_ACTIVE`, `GAME_MENU`, `GAME_WIN`), the `Keys[]` array, dimensions, level list, and raw pointers to the four module-level objects (Renderer, Player, Ball, Particles).

| Method | Responsibility |
|---|---|
| `Init()` | Load all shaders, textures, levels; create Player/Ball/Particles |
| `ProcessInput(dt)` | Move paddle left/right (A/D), release ball (Space) |
| `Update(dt)` | Advance ball, run collisions, update particles, check fall-off |
| `Render()` | Draw background → bricks → paddle → particles → ball |
| `DoCollisions()` | Ball vs. all bricks + ball vs. paddle |
| `ResetLevel()` | Reload current level file |
| `ResetPlayer()` | Return paddle and ball to start positions |

---

## 7. Collision Detection

Two types of collision are implemented in `Collision.h / .cpp`:

### 7.1 AABB–AABB (Rectangle vs. Rectangle)

```cpp
bool CheckCollision(GameObject& one, GameObject& two)
{
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
                      two.Position.x + two.Size.x >= one.Position.x;
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
                      two.Position.y + two.Size.y >= one.Position.y;
    return collisionX && collisionY;
}
```

The classic **Axis-Aligned Bounding Box** overlap test. Returns `true` if the two rectangles overlap on both axes simultaneously.

### 7.2 AABB–Circle (Brick vs. Ball)

Because the ball is circular, a more accurate method is used:

1. Find the ball's center point.
2. Find the AABB's center and half-extents.
3. Compute the **difference vector** from AABB center to ball center.
4. **Clamp** that difference to the AABB's half-extents — this gives the closest point on the AABB surface to the ball.
5. If the distance from that closest point to the ball center is less than the ball's radius → collision.

The function returns a `std::tuple<bool, Direction, glm::vec2>`:
- Whether a collision happened
- Which face of the brick was hit (UP, DOWN, LEFT, RIGHT)
- The penetration vector (how far the ball has overlapped)

### 7.3 Direction Classification (`VectorDirection`)

The hit direction is determined by dot-product comparison: the penetration vector is tested against four compass directions (`up`, `right`, `down`, `left`) and the one with the highest dot product (most aligned) is chosen.

### 7.4 Collision Response

**Brick collision:** If a non-solid brick is hit, it is marked `Destroyed = true`. The ball's velocity is flipped on the axis matching the hit direction, and its position is corrected by the penetration amount to prevent it sinking into the brick.

**Paddle collision:** More nuanced — the bounce angle depends on where on the paddle the ball hits. The further from the centre, the more horizontal the rebound. The outgoing velocity is normalised and rescaled to preserve the ball's original speed:

```cpp
float percentage = distance / (Player->Size.x / 2.0f);  // -1 to +1
Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
Ball->Velocity.y = -1.0f * std::abs(Ball->Velocity.y);  // always go up
Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
```

---

## 8. Particle System

The `ParticleGenerator` class implements a **fixed-size object pool** of 500 particles that trail behind the ball.

### How it works

1. **Pool initialisation** — 500 `Particle` structs are created with `Life = 0` (dead).
2. **Every frame** — `Update(dt, ball, 2)` is called:
   - It spawns 2 new particles per frame by finding a dead slot via `firstUnusedParticle()` (which remembers its last search position for O(1) amortised lookup).
   - Each new particle is "respawned" at the ball's position with a small random offset and the ball's velocity scaled to 10%.
   - All particles' `Life` is decremented by `dt`. Their positions drift opposite to their velocity (`position -= velocity * dt`), and their alpha fades at `2.5 × dt`.
3. **Rendering** — Only particles with `Life > 0` are drawn. Before drawing, `glBlendFunc(GL_SRC_ALPHA, GL_ONE)` enables **additive blending** — overlapping particles add their colours together, creating a bright glow rather than opaque stacking. After the particle draw, normal blending is restored.

---

## 9. Entry Point & Game Loop

`apps/sandbox/main.cpp` contains the complete lifecycle:

```
glfwInit() → glfwCreateWindow() → gladLoadGLLoader()
         → glEnable(GL_BLEND)
         → Breakout.Init()
         → while (!glfwWindowShouldClose):
               glfwPollEvents()
               Breakout.ProcessInput(dt)
               Breakout.Update(dt)
               glClear()
               Breakout.Render()
               glfwSwapBuffers()
         → glfwTerminate()
```

**Delta time** (`dt = currentFrame - lastFrame`) is measured each frame so that game speed is independent of frame rate — the ball moves the same distance per second whether the game runs at 30 FPS or 144 FPS.

Key input callbacks registered with GLFW:
- `key_callback` — fills `Breakout.Keys[]` on press/release; ESC closes the window.
- `framebuffer_size_callback` — calls `glViewport()` to adjust the rendering area if the window is resized.

---

## 10. Asset Pipeline

### Shaders
GLSL source files are loaded **at runtime** by `ResourceManager::loadShaderFromFile()` using `std::ifstream`. This means shader code can be edited without recompiling the C++ program.

| File | Purpose |
|---|---|
| `sprite.vs` / `sprite.fs` | All game objects (bricks, paddle, ball, background) |
| `particle.vs` / `particle.frag` | Ball trail particle effect |

### Textures
PNG and JPEG images are decoded by **stb_image** and uploaded to the GPU as `Texture2D` objects. Alpha-transparent PNGs (ball, paddle, particles) are loaded with `GL_RGBA`; opaque JPEGs use `GL_RGB`.

| File | Used for |
|---|---|
| `background.jpg` | Full-screen game background |
| `awesomeface.png` | The ball sprite |
| `block.png` | Destroyable bricks |
| `block_solid.png` | Indestructible bricks |
| `paddle.png` | Player paddle |
| `particle.png` | Trail particle sprite |

### Level Files
Four levels are stored as plain `.lvl` text files. Each row is a space-separated list of integers. Example (`one.lvl`):

```
1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1   ← solid top row
2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2   ← blue destroyable row
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0   ← empty row
3 3 3 0 0 3 3 3 3 0 0 3 3 3 3 0 0 3 3 3   ← green bricks with gaps
4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4   ← yellow full row
```

This data-driven approach means new levels can be designed in a text editor with no code changes.

---

## 11. Key Engineering Decisions & Lessons Learned

### 11.1 Modular Library Architecture
Reorganising the project from a flat layout into `libs/graphics`, `libs/game`, and `apps/sandbox` reinforced the principle of **separation of concerns**. The graphics layer knows nothing about game rules; the game layer is not tied to any specific application shell.

### 11.2 Include Path Namespacing
Headers are placed in `include/graphics/` and `include/game/` subdirectories. This means they are always included as `<graphics/Shader.h>` or `<game/Game.h>`, making it immediately obvious which library a header belongs to and preventing name collisions in larger projects.

### 11.3 The Working Directory Problem
A practical lesson in runtime file resolution: `std::ifstream` opens paths **relative to the process's working directory**, not relative to the binary's location. The CMake build was updated to place the `sandbox` binary at the **build root** and copy assets there, so `./sandbox` always finds `shaders/`, `textures/`, and `levels/` without specifying absolute paths.

### 11.4 OpenGL Object Ownership
Every GPU resource (shader program, texture, VBO, VAO) is explicitly created and deleted. The `ResourceManager::Clear()` call in `Game`'s destructor ensures no GPU memory leaks at shutdown. This is an important contrast to managed languages where resources are freed automatically.

### 11.5 The Particle Pool Pattern
Rather than allocating/freeing particle objects dynamically each frame (which would fragment the heap and stress the allocator), a fixed pool is preallocated. Dead particles are recycled. This is a classic real-time graphics pattern for achieving smooth, allocation-free gameplay.

### 11.6 Orthographic 2D Rendering on a 3D API
OpenGL is fundamentally a 3D API, but this project uses it purely in 2D by choosing an orthographic projection and fixing `z = 0`. This demonstrates that understanding the projection matrix is the key bridge between the 2D game world and the 3D GPU pipeline.

---

## 12. Known Limitations & Future Work

| Area | Current State | Possible Improvement |
|---|---|---|
| Game states | Only `GAME_ACTIVE` implemented | Add proper main menu and win screen |
| Lives system | One life; fall-off resets the level | Add a life counter (3 lives) |
| Score system | Not implemented | Track and display score per brick |
| Power-ups | Not implemented | Speed boost, sticky ball, laser paddle |
| Audio | Not implemented | Add OpenAL or miniaudio for sound |
| Resolution | Fixed 800×600 | Make window resizable with dynamic projection |
| Particle system | Tied to ball only | Abstract for general-purpose use |
| Level editor | Text file only | GUI or file-format-based level editor |

---

## 13. Build & Run Instructions

```bash
# Clone / navigate to project root
cd ~/Internship/glad/OpenGL_Project

# Configure (generates Makefiles in build/)
cmake -S . -B build

# Compile all targets
cmake --build build

# Run (must be from build/ so relative asset paths resolve)
cd build
./sandbox
```

**Controls:**
| Key | Action |
|---|---|
| `A` | Move paddle left |
| `D` | Move paddle right |
| `Space` | Release the ball from the paddle |
| `Esc` | Quit |

---

## 14. Dependency Summary

| Library | How included | Purpose |
|---|---|---|
| **GLAD** | Source bundled in `libs/external_libs/glad/` | OpenGL function pointer loader |
| **GLFW** | System library (`-lglfw`) | Window, context, input |
| **GLM** | Headers bundled in `libs/external_libs/glm/` | Vector/matrix math |
| **stb_image** | Single header in `libs/external_libs/stb_image/` | PNG/JPG image decoding |
| **OpenGL** | System library (`-lGL`) | GPU rendering API |

All third-party code is vendored (bundled) inside the repository except for GLFW and OpenGL, which are system-installed. This means the project can be built on any Linux machine with GLFW and an OpenGL-capable GPU driver without any additional downloads.

---

*End of Report*
