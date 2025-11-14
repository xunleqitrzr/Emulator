cmake_minimum_required(VERSION 3.10)

project(Emulator)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXE_SUFFIX "Debug")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(EXE_SUFFIX "Release")
else()
    set(EXE_SUFFIX "${CMAKE_BUILD_TYPE}")
endif()

# set C standard for C files
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

#set C++ standard for C++ files
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release" FORCE)
endif()

# add debugging symbols for debug builds
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2")

# --- Target 1: The Emulator (C)
# Defining C sources and headers
set(SOURCES
        src/main.c
        src/cpu.c
        src/ram.c
        src/bus.c
        src/rom.c
        src/fs/fs.c
)

set(HEADERS
        src/cpu.h
        src/ram.h
        src/bus.h
        src/rom.h
        src/fs/fs.h
)

# build Emulator from C files
add_executable(Emulator ${SOURCES} ${HEADERS})
# set output name to "EmulatorDebug" or "EmulatorRelease"
set_target_properties(Emulator PROPERTIES OUTPUT_NAME "Emulator${EXE_SUFFIX}")
