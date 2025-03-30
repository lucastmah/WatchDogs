# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(term_project ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(term_project LINK_PRIVATE hal)

# curl support
find_package(CURL REQUIRED)
target_link_libraries(term_project LINK_PRIVATE CURL::libcurl)

# GPIO support
find_library(GPIOD_LIBRARY gpiod)
target_link_libraries(term_project LINK_PRIVATE gpiod)

# ALSA support
find_package(ALSA REQUIRED)
target_link_libraries(term_project LINK_PRIVATE asound)

# PTHREAD support
find_package(Threads REQUIRED)
target_link_libraries(term_project LINK_PRIVATE Threads::Threads)

add_custom_command(TARGET term_project POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:term_project>"
     "~/cmpt433/public/myApps/term_project" 
  COMMENT "Copying ARM executable to public NFS directory")

# Copy cctv-server to NFS regardless of whether the build files were rebuilt
add_custom_target(copy_nodejs_files ALL
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
     "${CMAKE_SOURCE_DIR}/cctv-server"
     "~/cmpt433/public/myApps/cctv-server-copy" 
  COMMENT "Copying NodeJS server files to public NFS directory")
