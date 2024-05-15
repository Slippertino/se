cmake_minimum_required(VERSION 3.27)

project(tqp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)

set(TQP_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(TQP_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(TQP_THIRD_PARTY ${CMAKE_CURRENT_LIST_DIR}/third-party)

# Boost
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system thread atomic date_time REQUIRED)
#find_package(ICU COMPONENTS data i18n io test tu uc le lx REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(unofficial-cld3 CONFIG REQUIRED)

# Others
add_subdirectory(${TQP_THIRD_PARTY}/libstemmer)

set(BUILD_ICU ON)
set(ICU_BUILD_VERSION 61.1)
add_subdirectory(${TQP_THIRD_PARTY}/icu-cmake)

file(GLOB_RECURSE SOURCES ${TQP_INCLUDE_DIR}/* ${TQP_SOURCE_DIR}/*)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION ${CMAKE_BINARY_DIR})
set(TQP_DATA_DIR ${CMAKE_BINARY_DIR}/data)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${TQP_INCLUDE_DIR} ${TQP_THIRD_PARTY})
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${Boost_LIBRARIES}
    icu 
    stemmer
    unofficial::cld3::cld3
)

add_executable(tqp_main main.cpp ${SOURCES})
target_include_directories(tqp_main PUBLIC ${TQP_INCLUDE_DIR} ${TQP_THIRD_PARTY})
target_link_libraries(tqp_main PRIVATE
    ${Boost_LIBRARIES} 
    icu
    stemmer
    unofficial::cld3::cld3
)