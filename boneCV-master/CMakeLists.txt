cmake_minimum_required(VERSION 3.18)
project(test VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(${PROJECT_NAME} capture.c)
# add_library()

# find_library

add_custom_command(TARGET test POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
        "$<TARGET_FILE:test>"
        "~/cmpt433/public/myApps/test"
    COMMENT "Copying ARM executable to pubic NFS directory")


#cmake file in the making