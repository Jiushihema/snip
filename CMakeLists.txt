cmake_minimum_required(VERSION 3.16)

project(snip VERSION 1.0.0 LANGUAGES CXX)

# ── C++ 标准 ──────────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── 构建类型默认 Release ───────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ── 编译选项 ───────────────────────────────────────────────────────────────────
if(MSVC)
    add_compile_options(
        $<$<CONFIG:Release>:/O2>
        /W4
        /utf-8
    )
else()
    add_compile_options(
        $<$<CONFIG:Release>:-O2>
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# ── 自动收集源文件 ─────────────────────────────────────────────────────────────
file(GLOB_RECURSE SNIP_SOURCES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# ── 可执行目标 ─────────────────────────────────────────────────────────────────
add_executable(snip ${SNIP_SOURCES})

# ── Include 路径 ───────────────────────────────────────────────────────────────
target_include_directories(snip
    PRIVATE
        "${CMAKE_SOURCE_DIR}/src"
        "${CMAKE_SOURCE_DIR}/third_party"
)

# ── SQLiteCpp ──────────────────────────────────────────────────────────────────
set(SQLITECPP_RUN_CPPLINT      OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPCHECK     OFF CACHE BOOL "" FORCE)
set(SQLITECPP_BUILD_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(SQLITECPP_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(SQLITE_ENABLE_COLUMN_METADATA ON CACHE BOOL "" FORCE)

add_subdirectory(
    "${CMAKE_SOURCE_DIR}/third_party/SQLiteCpp"
    "${CMAKE_BINARY_DIR}/SQLiteCpp"
)

target_link_libraries(snip PRIVATE SQLiteCpp)

# ── curses 平台适配 ────────────────────────────────────────────────────────────
if(WIN32)
    set(PDCURSES_INCLUDE_DIR
        "${CMAKE_SOURCE_DIR}/third_party/pdcurses_dist/include")
    set(PDCURSES_LIBRARY
        "${CMAKE_SOURCE_DIR}/third_party/pdcurses_dist/lib/libpdcurses.a")

    target_include_directories(snip PRIVATE ${PDCURSES_INCLUDE_DIR})
    target_link_libraries(snip PRIVATE
        ${PDCURSES_LIBRARY}
        gdi32
        winmm
    )
else()
    find_package(Curses REQUIRED)
    target_link_libraries(snip PRIVATE ${CURSES_LIBRARIES})
    target_include_directories(snip PRIVATE ${CURSES_INCLUDE_DIR})
endif()

# ── 平台特定链接 ───────────────────────────────────────────────────────────────
if(WIN32)
    target_link_libraries(snip PRIVATE shlwapi ole32 oleaut32)
endif()

if(UNIX AND NOT APPLE)
    target_link_libraries(snip PRIVATE dl pthread)
endif()

# ── 安装规则 ───────────────────────────────────────────────────────────────────
install(TARGETS snip RUNTIME DESTINATION bin)
