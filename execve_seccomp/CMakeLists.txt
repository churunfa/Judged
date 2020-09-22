cmake_minimum_required(VERSION 2.5)
project(execve_seccomp)

set(CMAKE_CXX_STANDARD 14)

#add_executable(execve_seccomp src/main.cpp)

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/output)

set(CMAKE_C_FLAGS "-g -Wall -Werror -O3 -std=c99 -pie -fPIC")

file(GLOB SOURCE "src/*.c" "src/rules/*.c")
add_executable(sexec ${SOURCE})
target_link_libraries(sexec pthread seccomp)


install(FILES ${PROJECT_SOURCE_DIR}/output/sexec
        PERMISSIONS OWNER_EXECUTE OWNER_READ
        DESTINATION /usr/lib)
