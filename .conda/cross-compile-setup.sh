SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
source "${SCRIPT_DIR}/patch_conda_cxxflags.sh"
# Make sure the MPI wrappers find the right compilers
export OMPI_CC=$CC
export OMPI_CXX=$CXX
export OMPI_FC=$FC

if [ "$CROSS_COMPILE" = 1 ]; then
    CMAKE_PLATFORM_FLAGS+=(-DCMAKE_TOOLCHAIN_FILE="${SCRIPT_DIR}/cross-linux.cmake")
fi