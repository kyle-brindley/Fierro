#!/bin/bash -e

# Function to display the help message
show_help() {
    echo "Usage: source $(basename "$BASH_SOURCE") [OPTION]"
    echo "Valid options:"
    echo "  --fftw          : Build heffte fftw version"
    echo "  --cufft         : Build heffte cufft version"
    echo "  --rocfft        : Build heffte rocfft version"
    echo "  --help          : Display this help message"
    return 1
}

# Check for the number of arguments
if [ $# -ne 1 ]; then
    echo "Error: Please provide exactly one argument."
    show_help
    return 1
fi

# Check if the argument is a valid option
case "$1" in
    --fftw|--cufft|--rocfft)
        # Valid option
        selected_option="$1"
        # Create a new variable build_type by stripping "--" from selected_option
        build_type="${selected_option/--/}"
        ;;  
    --help)
        # Display help message
        show_help
        return 1
        ;;  
    *)  
        # Invalid option
        echo "Error: Invalid argument."
        show_help
        return 1
        ;;  
esac

# Now you can use $build_type in your code or build commands
echo "Heffte build type will be: $build_type"

# Determine the script's directory
SCRIPT_DIR=$(realpath "$(dirname "${BASH_SOURCE[0]}")")
echo "Script location: $SCRIPT_DIR"

# Determine the parent directory of the script's directory
PARENT_DIR=$(dirname $(dirname "${SCRIPT_DIR}"))

# Check if the 'heffte' directory exists in the parent directory; if not, clone it
HEFFTE_DIR="$PARENT_DIR/heffte"
if [ ! -d "$HEFFTE_DIR" ]; then
  echo "Directory 'heffte' does not exist in '$PARENT_DIR', downloading 'heffte'...."
  git clone https://github.com/icl-utk-edu/heffte.git "$HEFFTE_DIR"
else
  echo "Directory 'heffte' exists in '$PARENT_DIR', skipping 'heffte' download"
fi

# Define HeFFTe and FFTW directories
HEFFTE_SOURCE_DIR="$PARENT_DIR/heffte"
HEFFTE_INSTALL_DIR="$PARENT_DIR/heffte/install_heffte_$build_type"
HEFFTE_BUILD_DIR="$PARENT_DIR/heffte/build_heffte_$build_type"


# Configure heffte using CMake
cmake_options=(
    -D CMAKE_BUILD_TYPE=Release
    -D CMAKE_INSTALL_PREFIX="$HEFFTE_INSTALL_DIR"
    -D BUILD_SHARED_LIBS=ON
)

if [ "$build_type" == "fftw" ]; then
    cmake_options+=(
        -D Heffte_ENABLE_AVX=ON
        #-D Heffte_ENABLE_AVX512=ON
        -D Heffte_ENABLE_FFTW=ON
        #-D FFTW_ROOT="$FFTW_DIR"
    )
elif [ "$build_type" == "cufft" ]; then
    cmake_options+=(
        -D Heffte_ENABLE_CUDA=ON
        -D Heffte_DISABLE_GPU_AWARE_MPI=ON
    )
elif [ "$build_type" == "rocfft" ]; then
    cmake_options+=(
        -D CMAKE_CXX_COMPILER=hipcc
        -D Heffte_ENABLE_ROCM=ON
        -D Heffte_DISABLE_GPU_AWARE_MPI=ON
    )
fi

# Print CMake options for reference
echo "CMake Options: ${cmake_options[@]}"

# Configure HeFFTe
cmake "${cmake_options[@]}" -B "$HEFFTE_BUILD_DIR" -S "$HEFFTE_SOURCE_DIR"

# Build HeFFTe
echo "Building HeFFTe..."
make -C "$HEFFTE_BUILD_DIR" -j

# Install HeFFTe
echo "Installing HeFFTe..."
make -C "$HEFFTE_BUILD_DIR" install

echo "HeFFTe installation complete."

