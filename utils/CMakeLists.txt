cmake_minimum_required(VERSION 3.27)

project(seutils)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

set(SEUTILS_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(SEUTILS_THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third-party)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system thread atomic date_time url serialization REQUIRED)

find_package(OpenSSL REQUIRED)
find_package(amqpcpp CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
find_package(quill CONFIG REQUIRED)

set(BUILD_TEST OFF)
add_subdirectory(${SEUTILS_THIRD_PARTY}/libpqxx)
add_subdirectory(${SEUTILS_THIRD_PARTY}/yaml-cpp)
add_subdirectory(${SEUTILS_THIRD_PARTY}/libcds)
target_compile_options(cds-s INTERFACE -std=c++20)

file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/include/* 
    ${CMAKE_CURRENT_LIST_DIR}/src/*
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS} ${SEUTILS_THIRD_PARTY} ${SEUTILS_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${Boost_LIBRARIES} 
    amqpcpp
    quill::quill
    libpqxx::pqxx
    LibArchive::LibArchive
    OpenSSL::Crypto
    yaml-cpp::yaml-cpp
    cds-s
)