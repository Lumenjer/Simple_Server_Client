cmake_minimum_required(VERSION 3.22)
project(SERVER_CLIENT VERSION 0.1.0)
include(FetchContent)


set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

FetchContent_Declare(cli
    GIT_REPOSITORY https://github.com/daniele77/cli.git
)
FetchContent_MakeAvailable(cli)


include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)
find_package(Boost 1.78 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

if(WIN32)
  link_libraries(wsock32 ws2_32)
endif()

link_libraries(cli)

add_subdirectory(./projects/server)
add_subdirectory(./projects/client)
