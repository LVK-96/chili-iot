#!/bin/bash
set -e

IMAGE_NAME="chili-iot-build-env"

# Ensure Podman is installed
if ! command -v podman &> /dev/null; then
    echo "Error: Podman is not installed."
    echo "Please install Podman to build this project."
    exit 1
fi

CONTAINER_TOOL="podman"

# Configure run flags
# --userns=keep-id is required for rootless Podman to access host files with correct permissions
RUN_FLAGS=""
if [ "$CONTAINER_TOOL" = "podman" ]; then
    RUN_FLAGS="--userns=keep-id"
fi

# Build the image
echo "Building container image: $IMAGE_NAME using $CONTAINER_TOOL..."
# Using --network=host might help with downloads if DNS is flaky, but standard build is usually fine.
$CONTAINER_TOOL build -f Containerfile -t $IMAGE_NAME .

# Check if user wants to run a specific command or just drop into shell
if [ "$#" -eq 0 ]; then
    echo "Starting interactive shell in $IMAGE_NAME..."
    $CONTAINER_TOOL run --rm -it $RUN_FLAGS \
        -v "$(pwd):/workspace" \
        -u $(id -u):$(id -g) \
        $IMAGE_NAME
else
    echo "Running command in $IMAGE_NAME: $@"
    $CONTAINER_TOOL run --rm -it $RUN_FLAGS \
        -v "$(pwd):/workspace" \
        -u $(id -u):$(id -g) \
        $IMAGE_NAME \
        "$@"
fi
