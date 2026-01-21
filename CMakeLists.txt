cmake_minimum_required(VERSION 3.10)
project(Emulator C CXX)

# ---------------------------------------------------------
# Language standards
# ---------------------------------------------------------
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# Emulator (C)
# ---------------------------------------------------------
set(EMU_SOURCES
        src/main.c
        src/cpu.c
        src/ram.c
        src/bus.c
        src/rom.c
        src/gpu.c
        src/fs/fs.c
        src/security/security.c
)

set(EMU_HEADERS
        src/cpu.h
        src/ram.h
        src/bus.h
        src/rom.h
        src/gpu.h
        src/fs/fs.h
        src/security/security.h
)

add_executable(Emulator ${EMU_SOURCES} ${EMU_HEADERS})

# Debug macro (only for Debug builds)
target_compile_definitions(Emulator
        PRIVATE $<$<CONFIG:Debug>:DEBUG>
)

# Debug-only sanitizer flags
target_compile_options(Emulator
        PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>
)
target_link_options(Emulator
        PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>
)

# Release optimizations
target_compile_options(Emulator
        PRIVATE $<$<CONFIG:Release>:-O2>
)

# Output naming: EmulatorDebug / EmulatorRelease
set_target_properties(Emulator PROPERTIES
        OUTPUT_NAME "Emulator$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>"
)

# ---------------------------------------------------------
# Assembler (C++)
# ---------------------------------------------------------
add_executable(Assembler tools/assembler/main.cpp)

# Output name: easm
set_target_properties(Assembler PROPERTIES
        OUTPUT_NAME "easm"
)

# Release optimization
target_compile_options(Assembler PRIVATE
        $<$<CONFIG:Release>:-O2>
)

# ---------------------------------------------------------
# Optional: group in IDEs
# ---------------------------------------------------------
set_target_properties(Emulator PROPERTIES FOLDER "Emulator")
set_target_properties(Assembler PROPERTIES FOLDER "Tools")
