SET(T3_SRC
    Application.cpp
    Display.cpp
    Image.cpp
    Raytracer.cpp
    Scene.cpp
    Tarea3.cpp
)

add_executable(Tarea3 ${T3_SRC})
target_link_libraries(Tarea3 ${SDL_LIBRARY} ${OpenCL_LIB})

# Copy OpenCL Programs.
SET(CL_SRC
    CommonCL.hpp
    Matrix3.hpp
    Matrix4.hpp
    Noise.hpp
    Geometry.hpp
    VectorCL.hpp
    Raytracer.cl
)

set(CL_PREFIX ${Tarea3_BINARY_DIR}/dist/cl)
foreach(data_file ${CL_SRC})
    add_custom_command(
        OUTPUT "${CL_PREFIX}/${data_file}"
        COMMAND cmake -E copy
            "${CMAKE_CURRENT_SOURCE_DIR}/${data_file}"
            "${CL_PREFIX}/${data_file}"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/${data_file}"
    )
    list(APPEND data_file_dest "${CL_PREFIX}/${data_file}")
endforeach()
add_custom_target(CopyDataFiles ALL DEPENDS ${data_file_dest})
