C_COMPILER      ?= gcc
CXX_COMPILER	?= g++
CFLAGS          ?= -Wextra -Wall
BUILD_DIR_BASE  ?= build

default: release

.PHONY: all clean release debug build

all:
	$(MAKE) release
	$(MAKE) debug

release:
	$(MAKE) BUILD_TYPE=Release BUILD_DIR=$(BUILD_DIR_BASE)/Release build

debug:
	$(MAKE) BUILD_TYPE=Debug BUILD_DIR=$(BUILD_DIR_BASE)/Debug build

build:
	cmake -B $(BUILD_DIR) \
		-G 'Unix Makefiles' \
		-DCMAKE_C_COMPILER=$(C_COMPILER) \
		-DCMAKE_CXX_COMPILER=$(CXX_COMPILER) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_C_FLAGS="$(CFLAGS)" \
		-DCMAKE_CXX_FLAGS="$(CFLAGS)" \
		&& $(MAKE) -C $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR_BASE)
