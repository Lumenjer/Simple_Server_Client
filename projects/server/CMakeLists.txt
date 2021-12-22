cmake_minimum_required(VERSION 3.1.1)
project(server_echo VERSION 0.1.0)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

set(SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/serverUI.cpp
)

add_executable(server_echo ${SOURCES})
target_include_directories(
  server_echo
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
