# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(term_project ${MY_SOURCES})

# Make use of the HAL library
target_link_libraries(term_project LINK_PRIVATE hal)

find_library(GPIOD_LIBRARY gpiod)
target_link_libraries(term_project LINK_PRIVATE gpiod)

add_custom_command(TARGET term_project POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:term_project>"
     "~/cmpt433/public/myApps/term_project" 
  COMMENT "Copying ARM executable to public NFS directory")
