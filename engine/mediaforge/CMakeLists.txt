cmake_minimum_required(VERSION 3.25)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(MediaForgeEngine VERSION 0.1.0 LANGUAGES CXX)
  set(MEDIAFORGE_CXX_STANDARD 23 CACHE STRING "C++ standard used by MediaForge Engine")
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
  enable_testing()
elseif(NOT DEFINED MEDIAFORGE_CXX_STANDARD)
  set(MEDIAFORGE_CXX_STANDARD 23)
endif()

set(MEDIAFORGE_VERSION "0.2.0-dev")
set(MEDIAFORGE_SDL_VERSION "3.4.14" CACHE STRING "Exactly pinned stable SDL3 version")
set(MEDIAFORGE_SHADERC_VERSION "2026.1" CACHE STRING "Exactly pinned stable shaderc tool version")
option(MEDIAFORGE_FETCH_DEPENDENCIES "Fetch exact official dependency tags when system packages are unavailable" ON)
option(MEDIAFORGE_BUILD_TESTS "Build MediaForge headless tests" ON)
option(MEDIAFORGE_BUILD_GPU_SMOKE "Build the graphical SDL_GPU smoke executable" ON)
set(MEDIAFORGE_SDL3_INCLUDE_DIR "" CACHE PATH "Explicit SDL3 include directory for package-less toolchains")
set(MEDIAFORGE_SDL3_LIBRARY "" CACHE FILEPATH "Explicit SDL3 library for package-less toolchains")

add_library(mediaforge_project_options INTERFACE)
target_compile_features(mediaforge_project_options INTERFACE "cxx_std_${MEDIAFORGE_CXX_STANDARD}")
target_compile_options(mediaforge_project_options INTERFACE
  "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wall;-Wextra;-Wpedantic>"
  "$<$<AND:$<BOOL:${MEDIAFORGE_ENABLE_SANITIZERS}>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>>:-fsanitize=address,undefined;-fno-omit-frame-pointer>")
target_link_options(mediaforge_project_options INTERFACE
  "$<$<AND:$<BOOL:${MEDIAFORGE_ENABLE_SANITIZERS}>,$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>>:-fsanitize=address,undefined>")

if(MEDIAFORGE_SDL3_INCLUDE_DIR AND MEDIAFORGE_SDL3_LIBRARY)
  add_library(SDL3::SDL3 UNKNOWN IMPORTED)
  set_target_properties(SDL3::SDL3 PROPERTIES
    IMPORTED_LOCATION "${MEDIAFORGE_SDL3_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${MEDIAFORGE_SDL3_INCLUDE_DIR}")
  set(SDL3_VERSION "${MEDIAFORGE_SDL_VERSION}")
else()
  find_package(SDL3 ${MEDIAFORGE_SDL_VERSION} EXACT CONFIG QUIET)
  if(NOT SDL3_FOUND)
    if(NOT MEDIAFORGE_FETCH_DEPENDENCIES)
      message(FATAL_ERROR "SDL3 ${MEDIAFORGE_SDL_VERSION} not found. Install the exact devel package or enable MEDIAFORGE_FETCH_DEPENDENCIES.")
    endif()
    include(FetchContent)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(mediaforge_sdl3
      GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
      GIT_TAG release-${MEDIAFORGE_SDL_VERSION}
      GIT_SHALLOW TRUE
      SYSTEM)
    FetchContent_MakeAvailable(mediaforge_sdl3)
    set(SDL3_VERSION "${MEDIAFORGE_SDL_VERSION}")
  endif()
endif()

if(TARGET SDL3::SDL3-shared AND NOT TARGET SDL3::SDL3)
  add_library(SDL3::SDL3 ALIAS SDL3::SDL3-shared)
endif()
if(NOT TARGET SDL3::SDL3)
  message(FATAL_ERROR "SDL integration did not provide SDL3::SDL3")
endif()

message(STATUS "MediaForge Engine version: ${MEDIAFORGE_VERSION}")
message(STATUS "MediaForge C++ standard: C++${MEDIAFORGE_CXX_STANDARD}")
message(STATUS "MediaForge SDL version: ${MEDIAFORGE_SDL_VERSION}")
message(STATUS "MediaForge compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "MediaForge build type: ${CMAKE_BUILD_TYPE}")

add_library(mediaforge_foundation STATIC
  src/foundation/Log.cpp
  src/input/InputState.cpp)
target_include_directories(mediaforge_foundation PUBLIC include)
target_link_libraries(mediaforge_foundation PUBLIC mediaforge_project_options)
add_library(MediaForge::Foundation ALIAS mediaforge_foundation)

add_library(mediaforge_engine STATIC
  src/platform/Window.cpp
  src/input/Event.cpp
  src/render/GPUDevice.cpp
  src/render/Camera2D.cpp
  src/render/Image.cpp
  src/render/ParticleSystem2D.cpp
  src/render/RenderQueue2D.cpp
  src/render/Renderer2D.cpp
  src/ui/SegmentedLevelIndicator.cpp
  src/ui/Ui.cpp)
target_include_directories(mediaforge_engine PUBLIC include PRIVATE src)
target_link_libraries(mediaforge_engine PUBLIC mediaforge_foundation PRIVATE SDL3::SDL3)
add_library(MediaForge::Engine ALIAS mediaforge_engine)

get_target_property(MEDIAFORGE_ENGINE_LINKS mediaforge_engine LINK_LIBRARIES)
foreach(LINK_ITEM IN LISTS MEDIAFORGE_ENGINE_LINKS)
  if(LINK_ITEM MATCHES "(^|::)aegis")
    message(FATAL_ERROR "MediaForge target may not link AEGIS target: ${LINK_ITEM}")
  endif()
endforeach()

if(MEDIAFORGE_BUILD_GPU_SMOKE)
  find_program(MEDIAFORGE_GLSLC glslc)
  set(MEDIAFORGE_COMPILED_SHADERS)
  if(MEDIAFORGE_GLSLC)
    execute_process(COMMAND "${MEDIAFORGE_GLSLC}" --version OUTPUT_VARIABLE GLSLC_VERSION_OUTPUT)
    if(NOT GLSLC_VERSION_OUTPUT MATCHES "${MEDIAFORGE_SHADERC_VERSION}")
      message(FATAL_ERROR "glslc must be stable shaderc ${MEDIAFORGE_SHADERC_VERSION}: ${GLSLC_VERSION_OUTPUT}")
    endif()
    foreach(SHADER smoke.vert color.frag sprite.frag renderer.vert sprite_linear.frag composite.frag)
      set(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER}")
      set(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER}.spv")
      add_custom_command(OUTPUT "${OUTPUT}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shaders"
        COMMAND "${MEDIAFORGE_GLSLC}" --target-env=vulkan1.0 -o "${OUTPUT}" "${SOURCE}"
        DEPENDS "${SOURCE}" VERBATIM)
      list(APPEND MEDIAFORGE_COMPILED_SHADERS "${OUTPUT}")
    endforeach()
    add_custom_target(mediaforge_shaders DEPENDS ${MEDIAFORGE_COMPILED_SHADERS})
    set(MEDIAFORGE_SMOKE_SHADERS_AVAILABLE 1)
  else()
    message(WARNING "glslc ${MEDIAFORGE_SHADERC_VERSION} not found: GPU smoke builds as a diagnostic stub; install the exact stable tool and rebuild")
    set(MEDIAFORGE_SMOKE_SHADERS_AVAILABLE 0)
  endif()

  add_executable(mediaforge_gpu_smoke src/smoke/GpuSmoke.cpp)
  target_include_directories(mediaforge_gpu_smoke PRIVATE src)
  target_link_libraries(mediaforge_gpu_smoke PRIVATE mediaforge_engine SDL3::SDL3)
  target_compile_definitions(mediaforge_gpu_smoke PRIVATE
    MEDIAFORGE_SMOKE_SHADERS_AVAILABLE=${MEDIAFORGE_SMOKE_SHADERS_AVAILABLE}
    MEDIAFORGE_SHADER_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/shaders")
  if(TARGET mediaforge_shaders)
    add_dependencies(mediaforge_gpu_smoke mediaforge_shaders)
  endif()
endif()

if(MEDIAFORGE_BUILD_TESTS)
  add_executable(mediaforge_foundation_tests tests/FoundationTests.cpp)
  target_link_libraries(mediaforge_foundation_tests PRIVATE mediaforge_foundation)
  add_test(NAME mediaforge_foundation_tests COMMAND mediaforge_foundation_tests)
  add_executable(mediaforge_render_tests tests/RenderTests.cpp)
  target_link_libraries(mediaforge_render_tests PRIVATE mediaforge_engine)
  add_test(NAME mediaforge_render_tests COMMAND mediaforge_render_tests)
  add_test(NAME mediaforge_architecture_audit
    COMMAND ${CMAKE_COMMAND} -DMEDIAFORGE_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ArchitectureAudit.cmake)
endif()
