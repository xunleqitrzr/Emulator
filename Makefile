C_COMPILER      ?= gcc
CXX_COMPILER	?= g++
CFLAGS          ?= -Wextra -Wall
BUILD_DIR_BASE  ?= build
CPP_STD			?= c++17

default: release

.PHONY: all clean release debug build assembler

all: release

release:
	$(MAKE) BUILD_TYPE=Release BUILD_DIR=$(BUILD_DIR_BASE)/Release build

debug:
	$(MAKE) BUILD_TYPE=Debug BUILD_DIR=$(BUILD_DIR_BASE)/Debug build

build:
	cmake -B $(BUILD_DIR) \
			-G 'Unix Makefiles' \
			-DCMAKE_C_COMPILER=$(C_COMPILER) \
			-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
			-DCMAKE_C_FLAGS="$(CFLAGS)" \
			&& $(MAKE) -C $(BUILD_DIR)

assembler:
	mkdir -p $(BUILD_DIR_BASE)
	$(CXX_COMPILER) $(CFLAGS) -std=$(CPP_STD) ./tools/assembler/main.cpp -o ./build/easm -O2

clean:
	rm -rf $(BUILD_DIR_BASE)
