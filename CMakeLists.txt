cmake_minimum_required(VERSION 3.25)
project(AegisDominion LANGUAGES CXX)

set(
  AEGIS_CXX_STANDARD
  23
  CACHE STRING
  "C++ standard used by AEGIS DOMINION")
set_property(CACHE AEGIS_CXX_STANDARD PROPERTY STRINGS 23 26)

if(NOT AEGIS_CXX_STANDARD MATCHES "^(23|26)$")
  message(FATAL_ERROR "AEGIS_CXX_STANDARD must be 23 or 26 (requested: ${AEGIS_CXX_STANDARD})")
endif()

set(AEGIS_CXX_FEATURE "cxx_std_${AEGIS_CXX_STANDARD}")
if(NOT AEGIS_CXX_FEATURE IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  message(FATAL_ERROR
    "Compiler ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} does not support ${AEGIS_CXX_FEATURE}")
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(aegis_project_options INTERFACE)
target_compile_features(aegis_project_options INTERFACE ${AEGIS_CXX_FEATURE})
target_compile_options(aegis_project_options INTERFACE
  "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-Wall;-Wextra;-Wpedantic>")

find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

add_library(aegis_core
    src/core/GameLaunchConfig.cpp
    src/core/MapDocument.cpp
    src/core/PlayableMap.cpp
    src/editor/MapEditorModel.cpp)
target_include_directories(aegis_core PUBLIC src)
target_link_libraries(aegis_core PUBLIC aegis_project_options)

add_library(aegis_ui
    src/logging/Logger.cpp
    src/assets/AssetCatalog.cpp src/assets/AssetManager.cpp
    src/input/InputSystem.cpp
    src/ui/TextService.cpp src/ui/Theme.cpp src/ui/Layout.cpp src/ui/Interaction.cpp src/ui/UiRenderer.cpp
    src/animation/Tween.cpp src/animation/SpriteAnimation.cpp
    src/render/RenderContext.cpp
    src/settings/Settings.cpp)
target_include_directories(aegis_ui PUBLIC src)
target_link_libraries(aegis_ui PUBLIC aegis_project_options sfml-graphics sfml-window sfml-audio)

add_executable(aegis_dominion
    src/main.cpp
    src/app/Application.cpp src/app/ScreenManager.cpp
    src/screens/MainMenuScreen.cpp src/screens/GameScreen.cpp
    src/editor/EditorCamera.cpp src/editor/MapForgeScreen.cpp
    src/Assets.cpp src/Map.cpp src/Enemy.cpp src/Tower.cpp src/Projectile.cpp src/Effects.cpp src/WaveManager.cpp)
target_include_directories(aegis_dominion PRIVATE src)
target_link_libraries(aegis_dominion PRIVATE aegis_project_options aegis_core aegis_ui sfml-graphics sfml-window sfml-system sfml-audio)

add_executable(aegis_core_tests tests/map_document_tests.cpp)
target_link_libraries(aegis_core_tests PRIVATE aegis_project_options aegis_core)
target_compile_definitions(aegis_core_tests PRIVATE AEGIS_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_executable(aegis_editor_tests tests/editor_model_tests.cpp)
target_link_libraries(aegis_editor_tests PRIVATE aegis_project_options aegis_core)

add_executable(aegis_playtest_tests tests/playtest_conversion_tests.cpp)
target_link_libraries(aegis_playtest_tests PRIVATE aegis_project_options aegis_core)

add_executable(aegis_game_map_tests tests/game_map_projection_tests.cpp src/Map.cpp)
target_link_libraries(aegis_game_map_tests PRIVATE aegis_project_options aegis_core aegis_ui sfml-graphics sfml-window sfml-system)
target_include_directories(aegis_game_map_tests PRIVATE src)

add_executable(aegis_text_tests tests/text_service_tests.cpp)
target_link_libraries(aegis_text_tests PRIVATE aegis_project_options aegis_ui)
target_compile_definitions(aegis_text_tests PRIVATE AEGIS_SOURCE_DIR="${CMAKE_SOURCE_DIR}")

add_executable(aegis_foundation_tests tests/foundation_tests.cpp)
target_link_libraries(aegis_foundation_tests PRIVATE aegis_project_options aegis_ui)

enable_testing()
add_test(NAME aegis_core_tests COMMAND aegis_core_tests)
add_test(NAME aegis_editor_tests COMMAND aegis_editor_tests)
add_test(NAME aegis_playtest_tests COMMAND aegis_playtest_tests)
add_test(NAME aegis_game_map_tests COMMAND aegis_game_map_tests)
add_test(NAME aegis_text_tests COMMAND aegis_text_tests)
add_test(NAME aegis_foundation_tests COMMAND aegis_foundation_tests)

add_custom_command(TARGET aegis_dominion POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:aegis_dominion>/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/maps $<TARGET_FILE_DIR:aegis_dominion>/maps)
