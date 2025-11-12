C_COMPILER      ?= gcc
CFLAGS          ?= -Wextra -Wall
BUILD_DIR_BASE  ?= build

default: release

.PHONY: all clean release debug build

all: release

release:
	$(MAKE) BUILD_TYPE=Release BUILD_DIR=$(BUILD_DIR_BASE)/Release build

debug:
	$(MAKE) BUILD_TYPE=Debug BUILD_DIR=$(BUILD_DIR_BASE)/Debug build

build:
	cmake -S ./src \
		-B $(BUILD_DIR) \
		-G 'Unix Makefiles' \
		-DCMAKE_C_COMPILER=$(C_COMPILER) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_C_FLAGS="$(CFLAGS)" \
		&& $(MAKE) -C $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR_BASE)
