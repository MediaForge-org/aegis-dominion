if(NOT DEFINED MEDIAFORGE_SOURCE_DIR)
  message(FATAL_ERROR "MEDIAFORGE_SOURCE_DIR is required")
endif()

file(GLOB_RECURSE ENGINE_CODE
  "${MEDIAFORGE_SOURCE_DIR}/include/*"
  "${MEDIAFORGE_SOURCE_DIR}/src/*")

set(FORBIDDEN_PATTERNS
  "#include[ \t]*[<\"](src/|core/|editor/|screens/|Enemy|Tower|Wave|MapDocument)"
  "SFML|sf::"
  "Tower|Enemy|EnemyType|Wave|BossWave|AegisCore|SpawnGate|Credits|TowerUpgrade"
  "TargetingMode|MapForge|MapDocument|BuildZone|Verdant|Frost|Ash")

foreach(FILE_PATH IN LISTS ENGINE_CODE)
  if(IS_DIRECTORY "${FILE_PATH}")
    continue()
  endif()
  file(READ "${FILE_PATH}" CONTENT)
  foreach(PATTERN IN LISTS FORBIDDEN_PATTERNS)
    if(CONTENT MATCHES "${PATTERN}")
      message(FATAL_ERROR "MediaForge architecture violation in ${FILE_PATH}: ${PATTERN}")
    endif()
  endforeach()
endforeach()

file(READ "${MEDIAFORGE_SOURCE_DIR}/CMakeLists.txt" ENGINE_CMAKE)
if(ENGINE_CMAKE MATCHES "target_link_libraries\\([^\\)]*aegis")
  message(FATAL_ERROR "MediaForge must not link an AEGIS target")
endif()

message(STATUS "MediaForge architecture audit passed (${MEDIAFORGE_SOURCE_DIR})")
