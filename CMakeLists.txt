cmake_minimum_required(VERSION 3.0)
project(html-parser)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/lib")

include_directories(.)

set(LIB_SRC
    src/html_parser.cc
)

add_library(html_parser SHARED ${LIB_SRC})

set(LIBS
    html_parser
)

add_executable(html_parser_test tests/test.cc)
add_dependencies(html_parser_test ${LIBS})
target_link_libraries(html_parser_test ${LIBS})