cmake_minimum_required(VERSION 3.1.0)
project(SoftRenderer)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

# vulkan
include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${Vulkan_INCLUDE_DIR}/../Third-Party/Include)

# imgui/stb_image/assimp/ImGuiFileDialog
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ImGuiFileDialog)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/stb_image)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/assimp/include)

# walnut
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Appframe)

# source
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core/Tracer)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Core/Rasterizer)

# GLFW
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW)

set(IMGUI_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/imgui/backends/imgui_impl_vulkan.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/ImGuiFileDialog/ImGuiFileDialog.cpp
)

set (UI_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/Appframe/application.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Appframe/image.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Appframe/random.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/Appframe/input.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/interface.cpp 
)

set (CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CORE/renderer.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/CORE/camera.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/CORE/model.cpp
)

set (TRACER_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/CORE/Tracer/tracer.cpp 
)

set (RASTERIZER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CORE/Rasterizer/rasterizer.cpp 
)

add_executable(${PROJECT_NAME} main.cpp ${IMGUI_SOURCES} ${UI_SOURCES} ${CORE_SOURCES} ${TRACER_SOURCES} ${RASTERIZER_SOURCES})

set (ASSIMP_LIB
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/assimp/lib/Debug/assimp-vc143-mtd.lib
)

set (GLFW_LIB
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/GLFW/glfw3.lib
)

target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARY} ${GLFW_LIB} ${ASSIMP_LIB})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/assimp/bin/Debug/assimp-vc143-mtd.dll
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/
)