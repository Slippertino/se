cmake_minimum_required(VERSION 3.27)

project(crawler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

set(CRAWLER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(CRAWLER_THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third-party)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system thread atomic date_time url serialization REQUIRED)

#find_package(libpqxx CONFIG REQUIRED)
#find_package(LibCDS CONFIG REQUIRED)
#target_compile_options(LibCDS::cds-s INTERFACE -std=c++20)
find_package(OpenSSL REQUIRED)
find_package(amqpcpp CONFIG REQUIRED)
find_package(LibArchive REQUIRED)
find_package(quill CONFIG REQUIRED)

set(BUILD_TEST OFF)
add_subdirectory(${CRAWLER_THIRD_PARTY}/libpqxx)
set(BUILD_TEST OFF)

add_subdirectory(${CRAWLER_THIRD_PARTY}/libcds)
target_compile_options(cds INTERFACE -std=c++20)

set(ROBOTS_BUILD_STATIC OFF)
set(ROBOTS_INSTALL ON)
add_subdirectory(${CRAWLER_THIRD_PARTY}/robotstxt)

function(get_all_targets _result _dir)
    get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        get_all_targets(${_result} "${_subdir}")
    endforeach()

    get_directory_property(_sub_targets DIRECTORY "${_dir}" BUILDSYSTEM_TARGETS)
    set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()

get_all_targets(OUT ${CRAWLER_THIRD_PARTY}/robotstxt)
set_target_properties(${OUT} PROPERTIES CXX_STANDARD 17)

add_subdirectory(${CRAWLER_THIRD_PARTY}/certify)
add_subdirectory(${CRAWLER_THIRD_PARTY}/yaml-cpp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../html-analyzer htmlanalyzer)

file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/include/* 
    ${CMAKE_CURRENT_LIST_DIR}/src/*
)

file(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/crawler.yaml 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${CRAWLER_THIRD_PARTY} ${CRAWLER_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${Boost_LIBRARIES} 
    robots
    htmlanalyzer 
    cds
    amqpcpp
    certify::core
    libpqxx::pqxx
    quill::quill
    LibArchive::LibArchive
    OpenSSL::Crypto
    yaml-cpp::yaml-cpp
)