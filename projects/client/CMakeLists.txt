cmake_minimum_required(VERSION 3.1.1)
project(client_echo VERSION 0.1.0)

set(SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/clientUI.cpp
)

add_executable(client_echo ${SOURCES})

target_include_directories(
  client_echo
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
