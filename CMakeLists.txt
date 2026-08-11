cmake_minimum_required(VERSION 3.16)
project(AegisDominion LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

add_library(aegis_core
    src/core/MapDocument.cpp
    src/editor/MapEditorModel.cpp)
target_include_directories(aegis_core PUBLIC src)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(aegis_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(aegis_ui src/ui/TextService.cpp)
target_include_directories(aegis_ui PUBLIC src)
target_link_libraries(aegis_ui PUBLIC sfml-graphics)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(aegis_ui PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(aegis_dominion
    src/main.cpp src/Game.cpp src/Assets.cpp src/Map.cpp src/Enemy.cpp src/Tower.cpp src/Projectile.cpp src/Effects.cpp src/WaveManager.cpp)
target_include_directories(aegis_dominion PRIVATE src)
target_link_libraries(aegis_dominion PRIVATE aegis_core aegis_ui sfml-graphics sfml-window sfml-system sfml-audio)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(aegis_dominion PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(aegis_core_tests tests/map_document_tests.cpp)
target_link_libraries(aegis_core_tests PRIVATE aegis_core)
target_compile_definitions(aegis_core_tests PRIVATE AEGIS_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(aegis_core_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(aegis_text_tests tests/text_service_tests.cpp)
target_link_libraries(aegis_text_tests PRIVATE aegis_ui)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(aegis_text_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME aegis_core_tests COMMAND aegis_core_tests)
add_test(NAME aegis_text_tests COMMAND aegis_text_tests)

add_custom_command(TARGET aegis_dominion POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:aegis_dominion>/assets
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/maps $<TARGET_FILE_DIR:aegis_dominion>/maps)
