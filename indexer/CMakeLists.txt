cmake_minimum_required(VERSION 3.27)

project(indexer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

set(INDEXER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(INDEXER_THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third-party)

# Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system thread atomic date_time url serialization REQUIRED)

# TBB
find_package(TBB CONFIG REQUIRED)

# Quill
find_package(quill CONFIG REQUIRED)

# Others
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../html-analyzer htmlanalyzer)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../tqp tqp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../utils seutils)

file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/include/* 
    ${CMAKE_CURRENT_LIST_DIR}/src/*
)

file(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/indexer.yaml 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${INDEXER_THIRD_PARTY} ${INDEXER_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${Boost_LIBRARIES} 
    htmlanalyzer
    tqp
    seutils
    TBB::tbb
)