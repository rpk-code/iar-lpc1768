# HW details
BOARD_NAME := iar_lpc1768

# Download path for compiler and debugger
ARM_GCC_URL := https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2
SEGGER_JLINK_URL := https://www.segger.com/downloads/jlink/JLink_Linux_V794k_x86_64.tgz

# Docker build parameters
DOCKER_BUILD_CTX := ./env
DOCKERFILE := $(DOCKER_BUILD_CTX)/Dockerfile
DOCKER_BUILD_ARGS := \
	--build-arg COMPILER=$(ARM_GCC_URL) \
	--build-arg DEBUGGER=$(SEGGER_JLINK_URL)
# Docker build is quiet by default
# Set this variable as empty for verbose build
DOCKER_BUILD_QUIET := -q
DOCKER_IMAGE_NAME := $(BOARD_NAME)_build_img

# Docker container parameters
DOCKER_CONTAINER_NAME := $(BOARD_NAME)_build_cont
# Paths to be accessed in the docker container
CODE_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
# Docker volumes
DOCKER_COMMON_VOLUMES := \
	--volume=$(CODE_DIR):$(CODE_DIR) \
	--volume=/etc/passwd:/etc/passwd:ro
# Docker environment variables
# Docker arguments
DOCKER_COMMON_ARGS := \
	--rm \
	--tty \
	--privileged \
	--user=$(id -u):$(id -g) \
	--net=host \
	--workdir=$(CODE_DIR) \
	--name=$(DOCKER_CONTAINER_NAME) \
	$(DOCKER_COMMON_VOLUMES)

# Build parameters
# Build is quiet by default, set this to verbose when needed
BUILD_OUTPUT :=

all: build_docker_image
	@docker run $(DOCKER_COMMON_ARGS) $(DOCKER_IMAGE_NAME) ./build.sh compile $(BUILD_OUTPUT)
.PHONY: all

clean: build_docker_image
	@docker run $(DOCKER_COMMON_ARGS) $(DOCKER_IMAGE_NAME) ./build.sh clean
	@echo "Cleaned build directory"
.PHONY: clean

cleanall: build_docker_image
	@docker run $(DOCKER_COMMON_ARGS) $(DOCKER_IMAGE_NAME) rm -rf build
	@echo "Deleted build directory"
.PHONY: cleanall

flash: build_docker_image all
	@docker run $(DOCKER_COMMON_ARGS) $(DOCKER_IMAGE_NAME) ./debugger_scripts/flash.sh
.PHONY: flash

debug: build_docker_image all
	@docker run $(DOCKER_COMMON_ARGS) $(DOCKER_IMAGE_NAME) ./debugger_scripts/debugserver.sh &
	@sleep 2
	@docker exec -it ${DOCKER_CONTAINER_NAME} ./debugger_scripts/debug.sh
.PHONY: debug

dev_shell: build_docker_image
	@docker run --interactive $(DOCKER_COMMON_ARGS) $(DOCKER_IMAGE_NAME) /bin/bash
.PHONY: dev_shell

build_docker_image: $(DOCKERFILE)
	@docker build $(DOCKER_BUILD_QUIET) --tag=$(DOCKER_IMAGE_NAME) $(DOCKER_BUILD_ARGS) $(DOCKER_BUILD_CTX)
.PHONY: build_docker_image
