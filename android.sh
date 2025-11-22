#!/usr/bin/env bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check ANDROID_HOME
if [ -z "$ANDROID_HOME" ]; then
  echo -e "${RED}Error: ANDROID_HOME environment variable not set${NC}"
  exit 1
fi

# NDK configuration
NDK_VERSION="29.0.14206865"
NDK_PATH="${ANDROID_HOME}/ndk/${NDK_VERSION}"
API=29 # Android 10 minimum

# Check NDK
if [ ! -d "$NDK_PATH" ]; then
  echo -e "${RED}Error: NDK not found at: $NDK_PATH${NC}"
  exit 1
fi

# ABIs to build
# - arm64-v8a: Physical devices (99.9%+ Android 10+ device coverage)
# - x86_64: Android emulators
ABIS=("arm64-v8a" "x86_64")

echo -e "${GREEN}Building libmuslim for multiple Android ABIs...${NC}"

# Function to build for a specific ABI
build_abi() {
  local ABI=$1
  echo -e "${YELLOW}Building for ${ABI}...${NC}"

  # Configure CMake
  cmake -S . -B "build/android-${ABI}" \
    -DCMAKE_TOOLCHAIN_FILE="${NDK_PATH}/build/cmake/android.toolchain.cmake" \
    -DANDROID_ABI="${ABI}" \
    -DANDROID_PLATFORM="android-${API}" \
    -DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY="build/android/libs/${ABI}"

  # Build (only library target)
  cmake --build "build/android-${ABI}" --target muslim -j$(nproc)

  # Copy output
  mkdir -p "libs/${ABI}"
  cp "build/android-${ABI}/lib/libmuslim.so" "libs/${ABI}/"

  # Strip symbols to reduce size (30-40% reduction, no performance impact)
  echo -e "${YELLOW}Stripping symbols from ${ABI} library...${NC}"
  ${NDK_PATH}/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-strip "libs/${ABI}/libmuslim.so"

  echo -e "${GREEN}✓ ${ABI} build complete${NC}"
}

# Build for each ABI
for ABI in "${ABIS[@]}"; do
  build_abi "$ABI"
done

echo -e "${GREEN}All ABIs built successfully!${NC}"
echo -e "Libraries are in: libs/"
tree libs/ || ls -lR libs/
