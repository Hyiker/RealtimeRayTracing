# Projects Settings
cmake_minimum_required (VERSION 3.20)
project (RealtimeRayTracing)

set(TARGET rtrt)

# The main executable
add_executable(${TARGET}
  lib/glad/src/glad.c
  lib/imgui/backends/imgui_impl_glfw.cpp
  lib/imgui/backends/imgui_impl_opengl3.cpp
  src/tiny_obj_loader.h
  src/stb_image.h
  src/Helper.h
  src/Application.cpp
  src/Application.cpp
  src/MyApplication.cpp
  src/MyApplication.cpp
  src/glError.hpp
  src/glError.cpp
  src/main.cpp
  src/Camera.hpp
  src/Camera.cpp
  src/Shader.hpp
  src/Shader.cpp
  src/Quad.hpp
  src/Quad.cpp
  src/Cube.hpp
  src/Cube.cpp
  src/Scene.hpp
  src/Framebuffer.hpp
  src/GaussianBlur.hpp
  src/GaussianBlur.cpp
  src/Texture.hpp
  src/Scene.cpp
)

set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 17)
target_compile_options(${TARGET} PRIVATE -Wall)


add_subdirectory(lib/glfw EXCLUDE_FROM_ALL)
add_subdirectory(lib/glm EXCLUDE_FROM_ALL)
add_subdirectory(lib/imgui EXCLUDE_FROM_ALL)

target_include_directories(${TARGET} PRIVATE lib/glad/include lib/imgui/backends)

target_link_libraries(${TARGET}
  PRIVATE glfw
  PRIVATE glm
  PRIVATE IMGUI
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/asset.hpp.in
  ${CMAKE_CURRENT_BINARY_DIR}/src/asset.hpp
)
target_include_directories(${TARGET}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/src
)
