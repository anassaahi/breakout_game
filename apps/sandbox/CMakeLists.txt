add_executable(sandbox main.cpp)

# The sandbox executable needs the game library to run
target_link_libraries(sandbox PRIVATE game)

# Place the binary at the root of the build tree (build/) so it sits alongside
# the asset folders. Game.cpp opens files with bare paths ("shaders/sprite.vs",
# "textures/…", "levels/…") relative to CWD, so the CWD and assets must match.
set_target_properties(sandbox PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Copy each asset subfolder into the build root so paths resolve when you run
# the executable from build/ (e.g. ./sandbox or ./apps/sandbox/sandbox).
file(COPY ${CMAKE_SOURCE_DIR}/assets/shaders  DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/assets/textures DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/assets/levels   DESTINATION ${CMAKE_BINARY_DIR})