cmake_minimum_required(VERSION 3.27)

project(crawler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CRAWLER_THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third-party)

find_package(Boost COMPONENTS system thread atomic REQUIRED)
add_subdirectory(${CRAWLER_THIRD_PARTY}/robotstxt)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../html-analyzer htmlanalyzer)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*)
add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${CRAWLER_THIRD_PARTY})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES} robots htmlanalyzer)
