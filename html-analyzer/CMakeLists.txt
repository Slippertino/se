cmake_minimum_required(VERSION 3.27.1)

project(htmlanalyzer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HA_HEADERS_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(HA_SOURCES_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(HA_TESTS_DIR   ${CMAKE_CURRENT_LIST_DIR}/tests)

file(GLOB_RECURSE HA_SOURCES ${HA_SOURCES_DIR}/*)
file(GLOB_RECURSE HA_HEADERS ${HA_HEADERS_DIR}/*)

find_package(Boost COMPONENTS regex REQUIRED)
find_package(unofficial-gumbo CONFIG REQUIRED)

add_library(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${HA_SOURCES} ${HA_HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${HA_HEADERS_DIR} ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES} unofficial::gumbo::gumbo)

if (PROJECT_IS_TOP_LEVEL)
    add_executable(htmlanalyzer-main main.cpp ${HA_SOURCES} ${HA_HEADERS} ${Boost_INCLUDE_DIRS})
    target_include_directories(htmlanalyzer-main PUBLIC ${HA_HEADERS_DIR})
    target_link_libraries(htmlanalyzer-main PRIVATE unofficial::gumbo::gumbo)
endif(PROJECT_IS_TOP_LEVEL)