SET(TAREA2_SRC
    EventSource.cpp
    Element.cpp
    Game.cpp
    HardwareBuffer.cpp
    InputSystem.cpp
    Main.cpp
    Mesh.cpp
    Player.cpp
    Rectangle.cpp
    Renderer.cpp
    Sector.cpp
    Shader.cpp
    SimpleVertex.cpp
    System.cpp
)

add_executable(Tarea2 ${TAREA2_SRC})
target_link_libraries(Tarea2 ${SDL_LIBRARY} ${GLEW_LIB} ${OPENGL_gl_LIBRARY})

