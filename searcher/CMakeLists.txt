cmake_minimum_required(VERSION 3.14)

project(searcher)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_NO_WARN_NEW_VERSIONS 1)
set(BUILD_SHARED_LIBS ON)

set(SEARCHER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(SEARCHER_THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third-party)

# Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED)

# Userver
set(USERVER_FEATURE_CORE ON)
set(USERVER_FEATURE_POSTGRESQL ON)
set(USERVER_FEATURE_PATCH_LIBPQ OFF)
set(USERVER_FEATURE_RABBITMQ ON)
add_subdirectory(${SEARCHER_THIRD_PARTY}/userver)

# Others
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../tqp tqp)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../utils seutils)

file(GLOB_RECURSE SOURCES 
    ${CMAKE_CURRENT_LIST_DIR}/include/* 
    ${CMAKE_CURRENT_LIST_DIR}/src/*
)

file(GLOB CONFIG_FILES ${CMAKE_CURRENT_LIST_DIR}/configs/*)
foreach(CONFIG ${CONFIG_FILES})
    file(COPY ${CONFIG} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIRS} ${SEARCHER_THIRD_PARTY} ${SEARCHER_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${Boost_LIBRARIES}
    tqp
    userver::core 
    userver::postgresql
    userver::rabbitmq
    seutils
)