cmake_minimum_required(VERSION 3.19)
project(Acorn)

set(CMAKE_CXX_STANDARD 20)

add_executable(main main.cpp)
target_link_libraries(main ${CMAKE_CURRENT_LIST_DIR}/natives/linux-amd64/libskia.a
        ${CMAKE_CURRENT_LIST_DIR}/natives/linux-amd64/libskparagraph.a
        ${CMAKE_CURRENT_LIST_DIR}/natives/linux-amd64/libskshaper.a
        glfw ${CMAKE_DL_LIBS} pthread webp webpdemux webpmux jpeg freetype png z fontconfig GL harfbuzz harfbuzz-icu icuuc)
target_include_directories(main PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../skia)
target_compile_definitions(main PUBLIC SK_GL)
