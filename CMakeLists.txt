cmake_minimum_required(VERSION 3.0)

set (CMAKE_CXX_STANDARD 11)

project(ykchalresp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib") 
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

# the -I flag in gcc
include_directories(
  ${PROJECT_SOURCE_DIR}/include/
  /usr/include/ykpers-1
) 

# find boost program options
find_package(Boost 1.58 REQUIRED COMPONENTS program_options)

set(APP_SOURCES src/main.cpp src/LOG.cpp src/yubi.cpp src/prog_opts.cpp)
#set(LIB_SOURCES src/World.cpp)

# creates ./build/bin/runner
add_executable(${PROJECT_NAME} ${APP_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ykpers-1)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

# shared library
#set(LIBRARY_NAME World)
#add_library(${LIBRARY_NAME} SHARED ${LIB_SOURCES})
#target_link_libraries(${PROJECT_NAME} ${LIBRARY_NAME})