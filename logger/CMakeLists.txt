cmake_minimum_required(VERSION 3.27)

project(logger)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

set(LOGGER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(LOGGER_THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third-party)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system thread atomic date_time url serialization REQUIRED)
add_subdirectory(${LOGGER_THIRD_PARTY}/libcds)
target_compile_options(cds-s INTERFACE -std=c++20)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../utils seutils)

file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/include/* 
    ${CMAKE_CURRENT_LIST_DIR}/src/*
)

file(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/logger.yaml 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${LOGGER_THIRD_PARTY} ${LOGGER_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${Boost_LIBRARIES} 
    cds-s
    seutils
)