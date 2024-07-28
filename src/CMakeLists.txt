cmake_minimum_required(VERSION 3.1...3.27)

#-------------------------------------------------------------------------------
# Build Environment
#-------------------------------------------------------------------------------

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  add_definitions(-fpic -W -DMG_ENABLE_LINES -DMG_ENABLE_OPENSSL)
  add_compile_options(-W -Wall -Werror -Wextra -O3)
endif()

project(
  kasa
  VERSION 1.0
  LANGUAGES C
  )

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(kasa SHARED kasalua.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fpic")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
target_link_libraries(kasa liblua.so)
target_link_libraries(kasa m)
install(TARGETS kasa DESTINATION /usr/lib/lua/kasa/)
