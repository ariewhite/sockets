cmake_minimum_required(VERSION 3.10)

project(testovoe VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(Threads REQUIRED)

add_executable(client 
    headers/client.h
    src/client/client.cpp)
    
add_executable(server 
    headers/server.h
    src/server/server.cpp)


target_include_directories(client PUBLIC ${CMAKE_SOURCE_DIR}/headers)
target_include_directories(server PUBLIC ${CMAKE_SOURCE_DIR}/headers)

#target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT})
