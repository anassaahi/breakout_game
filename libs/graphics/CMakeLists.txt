# Compile all .cpp files in this folder into a static library called "graphics"
file(GLOB SOURCES "src/*.cpp")
add_library(graphics STATIC ${SOURCES} ../external_libs/glad/src/glad.c)

# Tell the graphics library where to find its headers
target_include_directories(graphics PUBLIC 
    include
    ../external_libs/glad/include
    ../external_libs/glm
    ../external_libs/stb_image/include
    ../game/include
)

# Link Ubuntu system libraries
target_link_libraries(graphics PUBLIC glfw GL X11 pthread Xrandr Xi dl)