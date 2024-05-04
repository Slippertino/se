cmake_minimum_required(VERSION 3.27)

project(tqp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TQP_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(TQP_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(TQP_THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third-party)

# Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system thread atomic date_time REQUIRED)
find_package(unofficial-cld3 CONFIG REQUIRED)

# Others
add_subdirectory(${TQP_THIRD_PARTY}/libstemmer)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../html-analyzer htmlanalyzer)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../utils seutils)

file(GLOB_RECURSE SOURCES ${TQP_INCLUDE_DIR}/* ${TQP_SOURCE_DIR}/*)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
set(TQP_DATA_DIR ${CMAKE_BINARY_DIR}/data)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${TQP_INCLUDE_DIR} ${TQP_THIRD_PARTY})
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Boost_LIBRARIES} 
    stemmer
    cld3
    seutils
)