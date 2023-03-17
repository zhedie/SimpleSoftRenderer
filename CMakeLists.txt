cmake_minimum_required(VERSION 3.1.0)
project(SimpleSoftRenderer)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(DEPENDENCY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependency")

include_directories(
    "${DEPENDENCY_PATH}"
    "${DEPENDENCY_PATH}/eigen3"
    "${DEPENDENCY_PATH}/imgui"
    "${DEPENDENCY_PATH}/GLFW"
    "${DEPENDENCY_PATH}/assimp"
    "${CAMKE_CURRENT_SOURCE_DIR}/src"
)

file(GLOB SRC_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stb_image.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SurroundCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Model.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Mesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Shader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/InputManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Renderer.cpp
)

file (GLOB IMGUI_SOURCES
    ${DEPENDENCY_PATH}/imgui/*.cpp
)

if (WIN32)
    # add_compile_options(/W4)
    add_compile_options(/wd4819)
endif()

add_executable(${PROJECT_NAME} 
    "${SRC_SOURCES}"
    "${IMGUI_SOURCES}"
)

target_link_libraries(
    ${PROJECT_NAME}
    "${DEPENDENCY_PATH}/GLFW/glfw3.lib"
    "${DEPENDENCY_PATH}/assimp/assimp-vc143-mtd.lib"
    "opengl32.lib"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/models
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${DEPENDENCY_PATH}/assimp/assimp-vc143-mtd.dll
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/
)