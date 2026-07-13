cmake_minimum_required(VERSION 3.16)
project(mini_redis)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

add_executable(mini_redis
    src/main.cpp
    src/server.cpp
    src/store.cpp
    src/resp.cpp
)

target_include_directories(mini_redis PRIVATE src)
target_link_libraries(mini_redis PRIVATE Threads::Threads)