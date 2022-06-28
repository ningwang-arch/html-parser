cmake_minimum_required(VERSION 3.0)
project(html-parser)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/lib")

include_directories(.)

find_package(LibXml2 REQUIRED)

if(LibXml2_FOUND)
    include_directories(${LIBXML2_INCLUDE_DIR})
    link_libraries(${LIBXML2_LIBRARIES})
endif()

set(LIB_SRC
    src/html_parser.cc
)

add_library(html_parser SHARED ${LIB_SRC})

set(LIBS
    html_parser
    ${LIBXML2_LIBRARIES}
)

add_executable(html_parser_test tests/test.cc)
add_dependencies(html_parser_test html_parser)
target_link_libraries(html_parser_test ${LIBS})