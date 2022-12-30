# CMake 3.25.0 and lower are broken with Ninja, MSVC, and PCH.
cmake_minimum_required(VERSION 3.25.1)
project(quirk)

add_library(quirk-headers INTERFACE)
target_include_directories(quirk-headers INTERFACE include)

##################
# Message Parser #
##################
option(QUIRK_BUILD_PARSER "Build message parser" ON)
if (QUIRK_BUILD_PARSER)
    add_subdirectory(src/message)
endif()

#####################
# Network Interface #
#####################
option(QUIRK_BUILD_NET "Build network layer" OFF)
if(QUIRK_BUILD_NET)
    add_subdirectory(src/net)
endif()

############
# Examples #
############
option(QUIRK_BUILD_EXAMPLES "Build example programs" ON)
if (QUIRK_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
