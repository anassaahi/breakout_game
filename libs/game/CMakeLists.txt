# Compile game logic into a static library
file(GLOB SOURCES "src/*.cpp")
add_library(game STATIC ${SOURCES})

# Game needs its own headers, AND it needs the graphics headers
target_include_directories(game PUBLIC include)
target_link_libraries(game PUBLIC graphics)