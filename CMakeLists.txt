cmake_minimum_required(VERSION 3.10)
project(QuarkEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SOURCE_FILES
    src/main.cpp
    src/editor.cpp
    src/scene.cpp
    src/camera.cpp
    src/ImGuizmo.cpp

    libs/imgui/imgui.cpp
    libs/imgui/imgui_draw.cpp
    libs/imgui/imgui_widgets.cpp
    libs/imgui/imgui_tables.cpp
    libs/imgui/backends/imgui_impl_opengl3.cpp

    libs/rlImGui/rlImGui.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
    src/headers
    libs/imgui
    libs/imgui/backends
    libs/rlImGui
    libs/raylib/include
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/raylib/lib
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        raylib
        winmm
        gdi32
        opengl32
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}
        raylib
        GL
        m
        pthread
        dl
        rt
        X11
    )
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME}
        raylib
        "-framework Cocoa"
        "-framework OpenGL"
        "-framework IOKit"
        "-framework CoreAudio"
        "-framework CoreVideo"
    )
endif()