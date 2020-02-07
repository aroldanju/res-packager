project(res-packager C)

add_definitions(-D__RES_DEBUG__)

file(GLOB SOURCES_RES src/*.h src/*.c)

add_library(${PROJECT_NAME} ${SOURCES_RES})
add_library(dependencies::res-packager ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src)

project(res-packager-sample C)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../sample)
add_executable(res-packager-sample sample/sample.c)
target_link_libraries(res-packager-sample dependencies::res-packager)