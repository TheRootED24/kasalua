cmake_minimum_required(VERSION 3.1...3.27)
 
project(
  kasa
  VERSION 1.0
  LANGUAGES C
  )

set(CMAKE_SHARED_LIBRARY_PREFIX "")
add_library(kasa SHARED kasalua.c)

target_link_libraries(kasa liblua.so)
target_link_libraries(kasa m)
install(TARGETS kasa DESTINATION /usr/lib/lua/kasa/)
