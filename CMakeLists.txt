###################################################################################
#                                                                                 #
# NAME: CMakeLsits.txt                                                            #
#                                                                                 #
# AUTHOR: Mike Karlesky, Mark VanderVoord, Greg Williams.                         #
# WRITTEN BY: Michael Brockus.                                                    #
#                                                                                 #
# License: MIT                                                                    #
#                                                                                 #
###################################################################################
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)


project(hunt LANGUAGES C DESCRIPTION "C Unit testing framework.")


add_subdirectory("src")


target_include_directories("hunt"
    PUBLIC 
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
        "$<INSTALL_INTERFACE:src>"

    PRIVATE "src"
)

add_library("hunt::framework" ALIAS "hunt")

install(TARGETS "hunt" EXPORT "huntConfig"
    ARCHIVE  DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_INSTALL_LIBDIR}"
    LIBRARY  DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_INSTALL_LIBDIR}"
    RUNTIME  DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_LIBDIR}")

install(DIRECTORY src/ DESTINATION src)
install(EXPORT huntConfig DESTINATION share/huntConfig/cmake)

# This makes the project importable from the build directory
export(TARGETS hunt FILE huntConfig.cmake)
