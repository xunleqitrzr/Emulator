cmake_minimum_required(VERSION 3.10)

project(Emulator)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(EXE_SUFFIX "Debug")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(EXE_SUFFIX "Release")
else()
    set(EXE_SUFFIX "${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../)

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

set(SOURCES
        main.c
        cpu.c
        ram.c
        bus.c
        rom.c
)

set(HEADERS
        cpu.h
        ram.h
        bus.h
        rom.h
)

add_executable(Emulator ${SOURCES} ${HEADERS})
set_target_properties(Emulator PROPERTIES OUTPUT_NAME "Emulator${EXE_SUFFIX}")
