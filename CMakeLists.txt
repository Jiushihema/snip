cmake_minimum_required(VERSION 3.16)
project(snip VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(third_party/SQLiteCpp)

include_directories(third_party)
include_directories(third_party/SQLiteCpp/include)
include_directories(src)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(snip ${SOURCES})

target_link_libraries(snip PRIVATE SQLiteCpp sqlite3)

if(MINGW)
    target_link_libraries(snip PRIVATE shlwapi)
endif()
